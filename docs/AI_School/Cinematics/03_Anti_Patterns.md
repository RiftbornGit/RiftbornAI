# 03 - Anti-Patterns

## 1. Moving The Camera Without Shot Purpose

Symptom: the camera orbits, dollies, or cranes constantly, but the viewer does not learn or feel anything new from the motion.

Why it fails: purposeless motion becomes visual noise. The viewer's attention is split between tracking the movement and reading the subject.

Do instead: ask what the motion reveals, emphasizes, or transitions. If the answer is nothing, hold the frame still.

## 2. Using Constant Motion Everywhere

Symptom: every shot has a slow push-in, orbit, or crane rise. No shot is static. The sequence feels restless but says nothing.

Why it fails: motion contrast is what creates energy. When everything moves, nothing stands out. Static framing can hold tension or clarity better than another dolly.

Do instead: vary motion deliberately. Use static shots for tension and stillness. Use motion for discovery and energy. The contrast creates rhythm.

## 3. Cutting Without New Information

Symptom: a cut changes the camera angle but does not change the emphasis, scale, emotional distance, or information density.

Why it fails: the viewer unconsciously expects every cut to deliver something new. A cut that changes nothing feels accidental or disorienting.

Do instead: each cut should change at least one meaningful dimension — angle, shot size, subject focus, or emotional distance.

## 4. Building Sequences Blindly

Symptom: editing a sequence without checking `get_sequencer_editor_context` or `list_sequence_bindings`. Tracks are added or modified based on assumption rather than editor state.

Why it fails: the sequence editor state may not match the assumed state. Bindings can be stale, missing, or pointed at the wrong actor. Editing blind produces drift.

Do instead: inspect the active Sequencer context before any modification. Confirm bindings before adding keys or adjusting tracks.

## 5. Assuming A Binding Exists Because The Actor Exists

Symptom: an actor is in the level, so the agent assumes it is correctly bound in the active sequence. Keys are added to what turns out to be a missing or stale binding.

Why it fails: actor presence in the level and binding presence in the sequence are independent states.

Do instead: use `assert_sequence_binding_exists` before operating on a binding. If the assertion fails, add the binding first with `add_sequence_track`.

## 6. Rendering Too Early

Symptom: `render_sequence` is called as the first review step. Every iteration takes a full render cycle. Problems are discovered only in the output video.

Why it fails: rendering is slow and should validate, not discover. Catching framing or continuity problems in a rendered video wastes minutes per fix.

Do instead: review shots in viewport first (`capture_viewport_sync`, `look_at_and_capture`). Render only when the shots already look clean in editor.

## 7. Overusing Lens Tricks Instead Of Fixing Staging

Symptom: subject is hard to read, so the response is extreme DOF, dramatic zoom, or lens flare. The underlying staging — actor position, silhouette, spatial relationship — was never addressed.

Why it fails: a beautiful lens cannot rescue weak blocking. If the subject is in the wrong place or has a bad silhouette, the shot will still feel unclear.

Do instead: fix the staging first. Move the actor, adjust the composition, improve the spatial relationship. Then choose a lens that supports the corrected staging.

## 8. Letting Every Shot Use The Same Framing Logic

Symptom: every shot is medium-wide, slightly above eye height, with gentle movement. The sequence has uniform visual character but no emphasis or dynamics.

Why it fails: cinematic rhythm requires variety. Wide shots establish, close shots emphasize, high angles diminish, low angles empower. Using one template throughout flattens every beat.

Do instead: vary shot size, angle, and movement to serve each beat's function. Contrast is how emphasis works.

## 9. Skipping Continuity Between Cuts

Symptom: after each cut, the viewer spends time reconstructing where everyone is, who is looking at what, and what changed. The sequence feels disjointed.

Why it fails: even stylized sequences need consistent spatial logic. When the viewer cannot maintain orientation, they disconnect from the story.

Do instead: maintain screen direction, eyeline consistency, and spatial logic across cuts. Review adjacent shots back-to-back to catch continuity breaks.

## Key Takeaway

Most weak cinematics are not caused by missing camera tools.

They are caused by missing shot purpose, missing binding proof, or missing review before render.
