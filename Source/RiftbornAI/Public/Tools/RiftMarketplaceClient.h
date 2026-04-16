// RiftMarketplaceClient.h
// HTTP client for the Tool Marketplace endpoints served by the RiftbornAI
// issuer Worker under /marketplace/*. All methods fire their callback on the
// game thread.

#pragma once

#include "CoreMinimal.h"
#include "Tools/RiftToolModel.h"

struct FRiftListingSummary
{
	FString ListingId;
	FString Slug;
	FString Title;
	FString Summary;
	FString DescriptionMd;   // long-form body for the detail panel
	FString Author;          // creator_id for now; display_name joined server-side later
	FString BlastRadius;
	int32   PriceCents = 0;
	FString Currency;
	FString LatestVersion;
	int32   Downloads = 0;
	int32   VoteScore = 0;   // sum of +1/-1 votes
	float   RatingAvg = -1.0f;  // -1 = no rating yet (vs. 0.0 which means all 0-stars)
	int32   RatingCount = 0;
	FString IconUrl;         // optional 256x256 thumbnail
	FString BannerUrl;       // optional wide hero image (featured rows)
	bool    bFeatured = false;
	int64   UpdatedAtSeconds = 0;  // Unix time of last change (for "Recent" sort)
	TArray<FString> Tags;
};

// Category (tag) bucket returned alongside listings so the left sidebar
// can render counts without a second round-trip.
struct FRiftCategoryCount
{
	FString Name;
	int32   Count = 0;
};

class RIFTBORNAI_API FRiftMarketplaceClient
{
public:
	static FString BaseUrl();

	/** Browse request parameters. All optional — defaults produce the
	 *  classic "popular, all prices, all tags" listing the Fab-style
	 *  front page shows on first open. */
	struct FBrowseParams
	{
		FString Query;
		FString Tag;
		FString PriceMode;   // "", "free", "paid"
		FString Sort;        // "", "popular", "recent", "price_asc", "price_desc", "rating"
		bool    bFeaturedOnly = false;
		int32   Limit = 30;
		int32   Offset = 0;
	};

	/** GET /marketplace/listings?q=&tag=&price=&sort=&featured=&limit=&offset= */
	static void Browse(const FBrowseParams& Params,
	                   TFunction<void(bool /*bOk*/,
	                                  const TArray<FRiftListingSummary>& /*Results*/,
	                                  const TArray<FRiftCategoryCount>&  /*Categories*/,
	                                  const FString& /*Err*/)> OnDone);

	/** Back-compat shim — older call sites pass only a search string. */
	static void Browse(const FString& Query,
	                   TFunction<void(bool /*bOk*/, const TArray<FRiftListingSummary>& /*Results*/, const FString& /*Err*/)> OnDone);

	/** GET /marketplace/listings/:id — pulls full bundle payload via a follow-up
	 *  request for FREE listings only; paid listings return the summary and the
	 *  caller must go through /checkout before /download. */
	static void InstallFree(const FString& ListingId,
	                        const FString& LicenseId,
	                        TFunction<void(bool, const FString& /*Err*/)> OnDone);

	/** POST /marketplace/checkout → Stripe Checkout URL the user opens in a browser. */
	static void CreateCheckout(const FString& ListingId,
	                           const FString& LicenseId,
	                           const FString& Email,
	                           TFunction<void(bool, const FString& /*Url*/, const FString& /*Err*/)> OnDone);

	/** GET /marketplace/download/:purchase_id?license_id=— called after Stripe success. */
	static void DownloadPurchase(const FString& PurchaseId,
	                             const FString& LicenseId,
	                             TFunction<void(bool, const FString& /*Err*/)> OnDone);

	/** POST /marketplace/listings — publish an authored tool. Server stamps
	 *  listing_id + signature + sha256; we write them back to disk on success. */
	static void Publish(const FRiftTool& Tool,
	                    const FString& CreatorId,
	                    const FString& LicenseId,
	                    TFunction<void(bool, const FString& /*Err*/)> OnDone);

	/** POST /marketplace/listings/:id/vote  body: {license_id, value:+1|-1} */
	static void Vote(const FString& ListingId, const FString& LicenseId, int32 Value,
	                 TFunction<void(bool, const FString& /*Err*/)> OnDone);

	/** POST /marketplace/listings/:id/report  reasons: spam|not_as_described|malware|ip|other */
	static void Report(const FString& ListingId, const FString& LicenseId,
	                   const FString& Reason, const FString& Body,
	                   TFunction<void(bool, const FString& /*Err*/)> OnDone);

	/** POST /marketplace/creators/onboard → Stripe onboarding URL. The
	 *  AcceptedTermsVersion must match what GET /legal/creator-terms returned
	 *  at the moment of consent, or the server refuses with 412. */
	static void OnboardCreator(const FString& LicenseId, const FString& DisplayName,
	                           const FString& AcceptedTermsVersion,
	                           TFunction<void(bool, const FString& /*Url*/, const FString& /*Err*/)> OnDone);

	/** GET /marketplace/legal/creator-terms → version + url + summary. */
	struct FLegalDescriptor { FString Kind; FString Version; FString Url; FString Summary; };
	static void FetchLegal(const FString& Which /*"refund"|"creator"*/,
	                       TFunction<void(bool, const FLegalDescriptor&, const FString& /*Err*/)> OnDone);

	/** GET /marketplace/purchases?license_id= → buyer's entitlement list. */
	struct FPurchase
	{
		FString PurchaseId;
		FString ListingId;
		FString Title;
		FString LatestVersion;
		int32   AmountCents = 0;
		FString Status;      // paid | refunded | chargeback
		FString Moderation;  // unreviewed | approved | flagged | banned
	};
	static void ListPurchases(const FString& LicenseId,
	                          TFunction<void(bool, const TArray<FPurchase>& /*Purchases*/, const FString& /*Err*/)> OnDone);

	/** Fetch every purchased tool that isn't present locally. Reports the
	 *  number installed and the number skipped (already present or failed). */
	struct FSyncReport { int32 Installed = 0; int32 Skipped = 0; int32 Failed = 0; FString FirstError; };
	static void SyncAll(const FString& LicenseId,
	                    TFunction<void(bool, const FSyncReport& /*Report*/)> OnDone);
};
