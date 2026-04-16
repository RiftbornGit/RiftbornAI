/**
 * RiftbornAI Copilot - Shared Core JavaScript
 * Common utilities, streaming, persistence, security, accessibility
 * @version 1.0.0
 */

(function(global) {
    'use strict';

    // ========== DOMPurify Inline (Minimal XSS Sanitizer) ==========
    const ALLOWED_TAGS = new Set([
        'a', 'b', 'i', 'u', 'em', 'strong', 'code', 'pre', 'p', 'br', 'hr',
        'ul', 'ol', 'li', 'blockquote', 'h1', 'h2', 'h3', 'h4', 'h5', 'h6',
        'span', 'div', 'table', 'thead', 'tbody', 'tr', 'th', 'td'
    ]);

    const ALLOWED_ATTRS = new Set([
        'href', 'target', 'rel', 'class', 'id', 'title', 'aria-label',
        'aria-hidden', 'role', 'tabindex', 'data-*'
    ]);

    const URL_PROTOCOLS = new Set(['http:', 'https:', 'mailto:']);

    function sanitizeHTML(html) {
        if (!html || typeof html !== 'string') return '';
        
        const template = document.createElement('template');
        template.innerHTML = html;
        
        const walker = document.createTreeWalker(
            template.content,
            NodeFilter.SHOW_ELEMENT,
            null,
            false
        );

        const nodesToRemove = [];
        
        while (walker.nextNode()) {
            const node = walker.currentNode;
            const tagName = node.tagName.toLowerCase();
            
            // Remove disallowed tags
            if (!ALLOWED_TAGS.has(tagName)) {
                nodesToRemove.push(node);
                continue;
            }

            // Clean attributes
            const attrs = Array.from(node.attributes);
            for (const attr of attrs) {
                const name = attr.name.toLowerCase();
                
                // Check data-* attributes
                if (name.startsWith('data-')) continue;
                
                if (!ALLOWED_ATTRS.has(name)) {
                    node.removeAttribute(attr.name);
                    continue;
                }

                // Sanitize href
                if (name === 'href') {
                    try {
                        const url = new URL(attr.value, window.location.origin);
                        if (!URL_PROTOCOLS.has(url.protocol)) {
                            node.removeAttribute('href');
                        }
                    } catch {
                        node.removeAttribute('href');
                    }
                }

                // Remove javascript: in any attribute
                if (attr.value.toLowerCase().includes('javascript:')) {
                    node.removeAttribute(attr.name);
                }
            }

            // Force target="_blank" links to have rel="noopener"
            if (tagName === 'a' && node.getAttribute('target') === '_blank') {
                node.setAttribute('rel', 'noopener noreferrer');
            }
        }

        nodesToRemove.forEach(n => n.parentNode?.removeChild(n));

        return template.innerHTML;
    }

    // ========== Streaming Response Handler ==========
    class StreamingHandler {
        constructor(options = {}) {
            this.onChunk = options.onChunk || (() => {});
            this.onComplete = options.onComplete || (() => {});
            this.onError = options.onError || (() => {});
            this.onMetrics = options.onMetrics || (() => {});
            
            this.buffer = '';
            this.totalTokens = 0;
            this.startTime = 0;
            this.firstTokenTime = 0;
        }

        async stream(url, body) {
            this.buffer = '';
            this.totalTokens = 0;
            this.startTime = performance.now();
            this.firstTokenTime = 0;

            try {
                const response = await fetch(url, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ ...body, stream: true })
                });

                if (!response.ok) {
                    throw new Error(`HTTP ${response.status}`);
                }

                const reader = response.body.getReader();
                const decoder = new TextDecoder();

                while (true) {
                    const { done, value } = await reader.read();
                    if (done) break;

                    const chunk = decoder.decode(value, { stream: true });
                    this.processChunk(chunk);
                }

                this.complete();

            } catch (error) {
                this.onError(error);
            }
        }

        processChunk(chunk) {
            // Handle Ollama's streaming format (newline-delimited JSON)
            const lines = (this.buffer + chunk).split('\n');
            this.buffer = lines.pop() || '';

            for (const line of lines) {
                if (!line.trim()) continue;

                try {
                    const data = JSON.parse(line);
                    
                    if (this.firstTokenTime === 0) {
                        this.firstTokenTime = performance.now();
                    }

                    if (data.message?.content) {
                        this.onChunk(data.message.content);
                        this.totalTokens += 1; // Approximate
                    }

                    if (data.done) {
                        if (data.eval_count) {
                            this.totalTokens = data.eval_count;
                        }
                        this.updateMetrics(data);
                    }
                } catch (e) {
                    // Non-JSON line, ignore
                }
            }
        }

        complete() {
            const totalTime = performance.now() - this.startTime;
            const ttft = this.firstTokenTime > 0 
                ? this.firstTokenTime - this.startTime 
                : totalTime;

            this.onMetrics({
                totalTime,
                ttft,
                tokens: this.totalTokens,
                tokensPerSecond: totalTime > 0 
                    ? (this.totalTokens / (totalTime / 1000)).toFixed(1) 
                    : 0
            });

            this.onComplete();
        }

        updateMetrics(data) {
            if (data.total_duration) {
                const secs = data.total_duration / 1e9;
                this.onMetrics({
                    totalTime: secs * 1000,
                    tokens: data.eval_count || this.totalTokens,
                    tokensPerSecond: data.eval_count 
                        ? (data.eval_count / secs).toFixed(1) 
                        : 0,
                    promptTokens: data.prompt_eval_count || 0,
                    contextLength: data.prompt_eval_count + (data.eval_count || 0)
                });
            }
        }
    }

    // ========== Session Persistence ==========
    class SessionStorage {
        constructor(storageKey = 'rb-copilot-session') {
            this.storageKey = storageKey;
            this.maxMessages = 500;
        }

        save(session) {
            try {
                const data = {
                    version: 1,
                    timestamp: Date.now(),
                    messages: session.messages.slice(-this.maxMessages),
                    model: session.model || '',
                    metadata: session.metadata || {}
                };
                localStorage.setItem(this.storageKey, JSON.stringify(data));
                return true;
            } catch (e) {
                console.warn('Session save failed:', e);
                return false;
            }
        }

        load() {
            try {
                const raw = localStorage.getItem(this.storageKey);
                if (!raw) return null;
                
                const data = JSON.parse(raw);
                if (data.version !== 1) return null;
                
                return {
                    messages: data.messages || [],
                    model: data.model || '',
                    metadata: data.metadata || {},
                    timestamp: data.timestamp
                };
            } catch (e) {
                console.warn('Session load failed:', e);
                return null;
            }
        }

        clear() {
            try {
                localStorage.removeItem(this.storageKey);
                return true;
            } catch (e) {
                return false;
            }
        }

        exportJSON() {
            const session = this.load();
            if (!session) return null;
            
            return JSON.stringify({
                exported: new Date().toISOString(),
                source: 'RiftbornAI Copilot',
                ...session
            }, null, 2);
        }

        exportMarkdown() {
            const session = this.load();
            if (!session) return null;

            let md = `# RiftbornAI Copilot Session\n`;
            md += `Exported: ${new Date().toISOString()}\n`;
            md += `Model: ${session.model || 'Unknown'}\n\n---\n\n`;

            for (const msg of session.messages) {
                const role = msg.role === 'user' ? '**You**' : '**RiftbornAI**';
                md += `### ${role}\n\n${msg.content}\n\n---\n\n`;
            }

            return md;
        }

        importJSON(jsonString) {
            try {
                const data = JSON.parse(jsonString);
                if (!Array.isArray(data.messages)) {
                    throw new Error('Invalid session format');
                }
                
                this.save({
                    messages: data.messages,
                    model: data.model || '',
                    metadata: { imported: true, originalTimestamp: data.timestamp }
                });
                
                return data;
            } catch (e) {
                throw new Error('Failed to import session: ' + e.message);
            }
        }
    }

    // ========== Connection Health Monitor ==========
    class ConnectionHealth {
        constructor(options = {}) {
            this.endpoint = options.endpoint || 'http://localhost:11434';
            this.pingInterval = options.pingInterval || 15000;
            this.onStatusChange = options.onStatusChange || (() => {});
            this.onMetadata = options.onMetadata || (() => {});
            
            this.status = 'unknown';
            this.uptime = 0;
            this.lastPing = 0;
            this.retryCount = 0;
            this.maxRetries = 3;
            this.offlineQueue = [];
            this.intervalId = null;
            this.metadata = {};
        }

        start() {
            this.check();
            this.intervalId = setInterval(() => this.check(), this.pingInterval);
        }

        stop() {
            if (this.intervalId) {
                clearInterval(this.intervalId);
                this.intervalId = null;
            }
        }

        async check() {
            const start = performance.now();
            
            try {
                const response = await fetch(`${this.endpoint}/api/tags`, {
                    method: 'GET',
                    signal: AbortSignal.timeout(5000)
                });

                if (response.ok) {
                    const data = await response.json();
                    this.lastPing = performance.now() - start;
                    this.retryCount = 0;
                    this.setStatus('online');
                    
                    this.metadata = {
                        models: data.models?.length || 0,
                        latency: Math.round(this.lastPing),
                        uptime: this.uptime
                    };
                    this.onMetadata(this.metadata);
                    
                    // Process offline queue
                    this.processQueue();
                } else {
                    this.handleFailure();
                }
            } catch (e) {
                this.handleFailure();
            }
        }

        handleFailure() {
            this.retryCount++;
            
            if (this.retryCount >= this.maxRetries) {
                this.setStatus('offline');
            } else {
                this.setStatus('reconnecting');
            }
        }

        setStatus(status) {
            if (this.status !== status) {
                this.status = status;
                this.onStatusChange(status);
            }
            
            if (status === 'online') {
                this.uptime += this.pingInterval / 1000;
            }
        }

        queueRequest(request) {
            if (this.offlineQueue.length < 50) {
                this.offlineQueue.push(request);
            }
        }

        async processQueue() {
            while (this.offlineQueue.length > 0 && this.status === 'online') {
                const request = this.offlineQueue.shift();
                try {
                    await request();
                } catch (e) {
                    console.warn('Queued request failed:', e);
                }
            }
        }
    }

    // ========== File Context Packer ==========
    class FilePacker {
        constructor(options = {}) {
            this.maxSize = options.maxSize || 100 * 1024; // 100KB default
            this.allowedTypes = options.allowedTypes || [
                'text/', 'application/json', 'application/javascript',
                'application/xml', 'application/x-yaml'
            ];
            this.onFile = options.onFile || (() => {});
            this.onError = options.onError || (() => {});
        }

        isAllowedType(file) {
            return this.allowedTypes.some(type => 
                file.type.startsWith(type) || file.name.endsWith('.txt') ||
                file.name.endsWith('.md') || file.name.endsWith('.py') ||
                file.name.endsWith('.cpp') || file.name.endsWith('.h') ||
                file.name.endsWith('.js') || file.name.endsWith('.ts') ||
                file.name.endsWith('.json') || file.name.endsWith('.yaml') ||
                file.name.endsWith('.yml')
            );
        }

        formatSize(bytes) {
            if (bytes < 1024) return bytes + ' B';
            if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + ' KB';
            return (bytes / (1024 * 1024)).toFixed(1) + ' MB';
        }

        async readFile(file) {
            if (!this.isAllowedType(file)) {
                this.onError(`File type not allowed: ${file.type || 'unknown'}`);
                return null;
            }

            if (file.size > this.maxSize) {
                this.onError(`File too large: ${this.formatSize(file.size)} (max ${this.formatSize(this.maxSize)})`);
                return null;
            }

            return new Promise((resolve, reject) => {
                const reader = new FileReader();
                
                reader.onload = (e) => {
                    const result = {
                        name: file.name,
                        size: file.size,
                        sizeFormatted: this.formatSize(file.size),
                        type: file.type,
                        content: e.target.result,
                        truncated: false
                    };
                    
                    this.onFile(result);
                    resolve(result);
                };
                
                reader.onerror = () => {
                    this.onError('Failed to read file');
                    reject(new Error('Read failed'));
                };
                
                reader.readAsText(file);
            });
        }

        packForContext(files) {
            let packed = '';
            let totalSize = 0;
            
            for (const file of files) {
                const header = `\n--- FILE: ${file.name} (${file.sizeFormatted}) ---\n`;
                const content = file.content;
                
                if (totalSize + header.length + content.length > this.maxSize) {
                    packed += header + '[Content truncated to fit context limit]\n';
                    break;
                }
                
                packed += header + content + '\n';
                totalSize += header.length + content.length;
            }
            
            return packed;
        }

        setupDropZone(element, options = {}) {
            const highlight = () => element.classList.add('dragover');
            const unhighlight = () => element.classList.remove('dragover');
            
            element.addEventListener('dragenter', (e) => {
                e.preventDefault();
                highlight();
            });
            
            element.addEventListener('dragover', (e) => {
                e.preventDefault();
                highlight();
            });
            
            element.addEventListener('dragleave', unhighlight);
            
            element.addEventListener('drop', async (e) => {
                e.preventDefault();
                unhighlight();
                
                const files = Array.from(e.dataTransfer.files);
                const results = [];
                
                for (const file of files) {
                    const result = await this.readFile(file);
                    if (result) results.push(result);
                }
                
                if (options.onDrop) {
                    options.onDrop(results);
                }
            });
        }
    }

    // ========== Accessibility Manager ==========
    class A11yManager {
        constructor() {
            this.reducedMotion = window.matchMedia('(prefers-reduced-motion: reduce)').matches;
            this.announcer = null;
            this.focusTrap = null;
            
            this.init();
        }

        init() {
            // Create live region for announcements
            this.announcer = document.createElement('div');
            this.announcer.setAttribute('aria-live', 'polite');
            this.announcer.setAttribute('aria-atomic', 'true');
            this.announcer.className = 'sr-only';
            document.body.appendChild(this.announcer);
            
            // Listen for motion preference changes
            window.matchMedia('(prefers-reduced-motion: reduce)').addEventListener('change', (e) => {
                this.reducedMotion = e.matches;
                document.body.classList.toggle('reduce-motion', this.reducedMotion);
            });
            
            // Keyboard navigation
            document.addEventListener('keydown', (e) => this.handleGlobalKeys(e));
        }

        announce(message, priority = 'polite') {
            if (!this.announcer) return;
            
            this.announcer.setAttribute('aria-live', priority);
            this.announcer.textContent = '';
            
            // Small delay to ensure screen readers pick up the change
            requestAnimationFrame(() => {
                this.announcer.textContent = message;
            });
        }

        handleGlobalKeys(e) {
            // Escape to close modals/panels
            if (e.key === 'Escape') {
                const modal = document.querySelector('.modal-overlay.visible');
                if (modal) {
                    modal.classList.remove('visible');
                    this.releaseFocusTrap();
                    e.preventDefault();
                }
            }
        }

        trapFocus(container) {
            const focusable = container.querySelectorAll(
                'button, [href], input, select, textarea, [tabindex]:not([tabindex="-1"])'
            );
            
            if (focusable.length === 0) return;
            
            const first = focusable[0];
            const last = focusable[focusable.length - 1];
            
            this.focusTrap = (e) => {
                if (e.key !== 'Tab') return;
                
                if (e.shiftKey) {
                    if (document.activeElement === first) {
                        e.preventDefault();
                        last.focus();
                    }
                } else {
                    if (document.activeElement === last) {
                        e.preventDefault();
                        first.focus();
                    }
                }
            };
            
            container.addEventListener('keydown', this.focusTrap);
            first.focus();
        }

        releaseFocusTrap() {
            if (this.focusTrap) {
                document.removeEventListener('keydown', this.focusTrap);
                this.focusTrap = null;
            }
        }

        setFocusRingStyle(style = 'visible') {
            document.body.dataset.focusStyle = style;
        }
    }

    // ========== Message Actions ==========
    class MessageActions {
        constructor(options = {}) {
            this.onCopy = options.onCopy || (() => {});
            this.onRetry = options.onRetry || (() => {});
            this.onEdit = options.onEdit || (() => {});
            this.onPin = options.onPin || (() => {});
            
            this.pinnedMessages = new Set();
        }

        createToolbar(message, role) {
            const toolbar = document.createElement('div');
            toolbar.className = 'message-actions';
            toolbar.setAttribute('role', 'toolbar');
            toolbar.setAttribute('aria-label', 'Message actions');
            
            const actions = [
                { icon: 'copy', title: 'Copy message', action: () => this.copy(message) },
                ...(role === 'user' ? [
                    { icon: 'edit', title: 'Edit and resend', action: () => this.edit(message) }
                ] : [
                    { icon: 'retry', title: 'Regenerate response', action: () => this.retry(message) }
                ]),
                { icon: 'pin', title: 'Pin message', action: () => this.pin(message), toggle: true }
            ];

            for (const action of actions) {
                const btn = document.createElement('button');
                btn.className = 'message-action-btn';
                btn.setAttribute('title', action.title);
                btn.setAttribute('aria-label', action.title);
                btn.innerHTML = this.getIcon(action.icon);
                btn.addEventListener('click', (e) => {
                    e.stopPropagation();
                    action.action();
                    if (action.toggle) {
                        btn.classList.toggle('pinned');
                    }
                });
                toolbar.appendChild(btn);
            }

            return toolbar;
        }

        getIcon(name) {
            const icons = {
                copy: '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="9" y="9" width="13" height="13" rx="2"/><path d="M5 15H4a2 2 0 01-2-2V4a2 2 0 012-2h9a2 2 0 012 2v1"/></svg>',
                edit: '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M11 4H4a2 2 0 00-2 2v14a2 2 0 002 2h14a2 2 0 002-2v-7"/><path d="M18.5 2.5a2.121 2.121 0 013 3L12 15l-4 1 1-4 9.5-9.5z"/></svg>',
                retry: '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="1 4 1 10 7 10"/><path d="M3.51 15a9 9 0 102.13-9.36L1 10"/></svg>',
                pin: '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 17v5"/><path d="M9 10.76a2 2 0 01-1.11 1.79l-1.78.9A2 2 0 005 15.24V16a1 1 0 001 1h12a1 1 0 001-1v-.76a2 2 0 00-1.11-1.79l-1.78-.9A2 2 0 0115 10.76V7a1 1 0 00-1-1h-4a1 1 0 00-1 1z"/></svg>'
            };
            return icons[name] || '';
        }

        async copy(message) {
            const text = message.querySelector('.message-body, .message-content')?.textContent || '';
            try {
                await navigator.clipboard.writeText(text);
                this.onCopy(true);
            } catch {
                // Fallback
                const ta = document.createElement('textarea');
                ta.value = text;
                document.body.appendChild(ta);
                ta.select();
                document.execCommand('copy');
                ta.remove();
                this.onCopy(true);
            }
        }

        retry(message) {
            const prevUserMsg = message.previousElementSibling;
            if (prevUserMsg?.classList.contains('user')) {
                const text = prevUserMsg.querySelector('.message-body, .message-content')?.textContent;
                this.onRetry(text);
            }
        }

        edit(message) {
            const text = message.querySelector('.message-body, .message-content')?.textContent || '';
            this.onEdit(text, message);
        }

        pin(message) {
            const id = message.dataset.id || Date.now().toString();
            message.dataset.id = id;
            
            if (this.pinnedMessages.has(id)) {
                this.pinnedMessages.delete(id);
                message.classList.remove('pinned');
            } else {
                this.pinnedMessages.add(id);
                message.classList.add('pinned');
            }
            
            this.onPin(Array.from(this.pinnedMessages));
        }
    }

    // ========== Virtual List (Performance) ==========
    class VirtualList {
        constructor(container, options = {}) {
            this.container = container;
            this.itemHeight = options.itemHeight || 100;
            this.buffer = options.buffer || 5;
            this.items = [];
            this.renderItem = options.renderItem || ((item) => item);
            
            this.viewport = null;
            this.content = null;
            this.startIndex = 0;
            this.endIndex = 0;
            
            this.init();
        }

        init() {
            this.viewport = document.createElement('div');
            this.viewport.style.cssText = 'overflow-y: auto; height: 100%;';
            
            this.content = document.createElement('div');
            this.content.style.position = 'relative';
            
            this.viewport.appendChild(this.content);
            this.container.appendChild(this.viewport);
            
            this.viewport.addEventListener('scroll', () => this.onScroll());
            
            new ResizeObserver(() => this.render()).observe(this.viewport);
        }

        setItems(items) {
            this.items = items;
            this.content.style.height = `${items.length * this.itemHeight}px`;
            this.render();
        }

        addItem(item) {
            this.items.push(item);
            this.content.style.height = `${this.items.length * this.itemHeight}px`;
            this.render();
            this.scrollToBottom();
        }

        onScroll() {
            this.render();
        }

        render() {
            const scrollTop = this.viewport.scrollTop;
            const viewportHeight = this.viewport.clientHeight;
            
            this.startIndex = Math.max(0, Math.floor(scrollTop / this.itemHeight) - this.buffer);
            this.endIndex = Math.min(
                this.items.length,
                Math.ceil((scrollTop + viewportHeight) / this.itemHeight) + this.buffer
            );

            // Clear and re-render visible items
            this.content.innerHTML = '';
            
            for (let i = this.startIndex; i < this.endIndex; i++) {
                const item = this.items[i];
                const el = this.renderItem(item, i);
                el.style.position = 'absolute';
                el.style.top = `${i * this.itemHeight}px`;
                el.style.left = '0';
                el.style.right = '0';
                this.content.appendChild(el);
            }
        }

        scrollToBottom() {
            this.viewport.scrollTop = this.viewport.scrollHeight;
        }
    }

    // ========== Toast Notifications ==========
    class ToastManager {
        constructor() {
            this.container = document.createElement('div');
            this.container.className = 'toast-container';
            this.container.style.cssText = `
                position: fixed;
                top: 80px;
                right: 20px;
                display: flex;
                flex-direction: column;
                gap: 8px;
                z-index: 1001;
                pointer-events: none;
            `;
            document.body.appendChild(this.container);
        }

        show(message, type = 'info', duration = 3000) {
            const toast = document.createElement('div');
            toast.className = `toast ${type}`;
            toast.style.cssText = `
                padding: 10px 16px;
                border-radius: 10px;
                background: var(--panel-3);
                border: 1px solid var(--border);
                color: var(--text-1);
                font-size: 14px;
                pointer-events: auto;
                opacity: 0;
                transform: translateX(20px);
                transition: opacity 0.2s, transform 0.2s;
            `;
            
            if (type === 'error') {
                toast.style.borderColor = 'rgba(255, 107, 107, 0.6)';
                toast.style.color = 'var(--error)';
            } else if (type === 'success') {
                toast.style.borderColor = 'rgba(123, 215, 165, 0.6)';
                toast.style.color = 'var(--success)';
            }
            
            toast.textContent = message;
            this.container.appendChild(toast);
            
            // Animate in
            requestAnimationFrame(() => {
                toast.style.opacity = '1';
                toast.style.transform = 'translateX(0)';
            });
            
            // Remove after duration
            setTimeout(() => {
                toast.style.opacity = '0';
                toast.style.transform = 'translateX(20px)';
                setTimeout(() => toast.remove(), 200);
            }, duration);
        }
    }

    // ========== Export API ==========
    global.RiftbornCopilot = {
        sanitizeHTML,
        StreamingHandler,
        SessionStorage,
        ConnectionHealth,
        FilePacker,
        A11yManager,
        MessageActions,
        VirtualList,
        ToastManager,
        
        // Utility functions
        formatTime: (date) => date.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' }),
        formatDuration: (ms) => {
            const s = Math.floor(ms / 1000);
            const m = Math.floor(s / 60);
            return `${String(m).padStart(2, '0')}:${String(s % 60).padStart(2, '0')}`;
        },
        debounce: (fn, delay) => {
            let timer;
            return (...args) => {
                clearTimeout(timer);
                timer = setTimeout(() => fn(...args), delay);
            };
        }
    };

})(typeof window !== 'undefined' ? window : this);
