// Copyright 2026 RiftbornAI. All Rights Reserved.
//
// RiskTierMapping.h — Canonical mapping between the three risk tier enums.
//
// THREE ENUMS EXIST (tech debt — should be consolidated to one):
//   ERiftbornRiskTier (ExecCtxValidator.h) — crypto validation, matches Python
//   EContractRiskTier (ToolContract.h) — contract parsing from JSON
//   EToolRisk (ClaudeToolUse_Types.h) — tool registry classification
//
// This header provides safe conversion functions between them.
// Long-term: migrate everything to ERiftbornRiskTier and delete the others.

#pragma once

#include "Governance/ExecCtxValidator.h"
#include "Governance/ToolContract.h"
#include "ClaudeToolUse_Types.h"

namespace RiftbornRiskTierMapping
{

/** Convert EContractRiskTier → ERiftbornRiskTier (contract → crypto validation) */
inline ERiftbornRiskTier FromContract(EContractRiskTier ContractTier)
{
	switch (ContractTier)
	{
		case EContractRiskTier::Safe:                 return ERiftbornRiskTier::Safe;
		case EContractRiskTier::Verification:         return ERiftbornRiskTier::Safe;
		case EContractRiskTier::MutatingReversible:   return ERiftbornRiskTier::MutatingReversible;
		case EContractRiskTier::MutatingIrreversible: return ERiftbornRiskTier::MutatingProject;
		case EContractRiskTier::Dangerous:            return ERiftbornRiskTier::Destructive; // Fail-closed: Dangerous → highest mutable tier
		case EContractRiskTier::Destructive:          return ERiftbornRiskTier::Destructive;
		default:                                      return ERiftbornRiskTier::Unknown;
	}
}

/** Convert ERiftbornRiskTier → EContractRiskTier (crypto → contract) */
inline EContractRiskTier ToContract(ERiftbornRiskTier RiftbornTier)
{
	switch (RiftbornTier)
	{
		case ERiftbornRiskTier::Safe:               return EContractRiskTier::Safe;
		case ERiftbornRiskTier::Recovery:            return EContractRiskTier::MutatingReversible; // No Recovery in contract enum
		case ERiftbornRiskTier::MutatingReversible:  return EContractRiskTier::MutatingReversible;
		case ERiftbornRiskTier::MutatingProject:     return EContractRiskTier::MutatingIrreversible;
		case ERiftbornRiskTier::Destructive:         return EContractRiskTier::Destructive;
		default:                                     return EContractRiskTier::Unknown;
	}
}

/** Convert ERiftbornRiskTier → EToolRisk (crypto → registry) */
inline EToolRisk ToToolRisk(ERiftbornRiskTier RiftbornTier)
{
	switch (RiftbornTier)
	{
		case ERiftbornRiskTier::Safe:               return EToolRisk::Safe;
		case ERiftbornRiskTier::Recovery:            return EToolRisk::Elevated;
		case ERiftbornRiskTier::MutatingReversible:  return EToolRisk::Mutation;
		case ERiftbornRiskTier::MutatingProject:     return EToolRisk::Mutation;
		case ERiftbornRiskTier::Destructive:         return EToolRisk::Destructive;
		default:                                     return EToolRisk::Unknown;
	}
}

/** Convert EToolRisk → ERiftbornRiskTier (registry → crypto) */
inline ERiftbornRiskTier FromToolRisk(EToolRisk ToolRisk)
{
	switch (ToolRisk)
	{
		case EToolRisk::Safe:        return ERiftbornRiskTier::Safe;
		case EToolRisk::Elevated:    return ERiftbornRiskTier::Recovery;
		case EToolRisk::Mutation:    return ERiftbornRiskTier::MutatingReversible;
		case EToolRisk::Dangerous:   return ERiftbornRiskTier::MutatingProject;
		case EToolRisk::Destructive: return ERiftbornRiskTier::Destructive;
		default:                     return ERiftbornRiskTier::Unknown;
	}
}

/** Convert EContractRiskTier → EToolRisk (contract → registry, replaces FToolContract::ToToolRisk()) */
inline EToolRisk ContractToToolRisk(EContractRiskTier ContractTier)
{
	return ToToolRisk(FromContract(ContractTier));
}

} // namespace RiftbornRiskTierMapping
