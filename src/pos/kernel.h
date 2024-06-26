// Copyright (c) 2011-2013 The PPCoin developers
// Copyright (c) 2013-2014 The NovaCoin Developers
// Copyright (c) 2014-2018 The BlackCoin Developers
// Copyright (c) 2015-2019 The PIVX developers
// Copyright (c) 2018-2020 The Ion developers
// Copyright (c) 2022 The Wagerr developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_KERNEL_H
#define BITCOIN_KERNEL_H

#include "validation.h"
#include "stakeinput.h"


// MODIFIER_INTERVAL: time to elapse before new modifier is computed
static const unsigned int MODIFIER_INTERVAL = 60;

// MODIFIER_INTERVAL_RATIO:
// ratio of group interval length between the last group and the first group
static const int MODIFIER_INTERVAL_RATIO = 3;

// Compute the hash modifier for proof-of-stake
bool GetKernelStakeModifier(const uint256& hashBlockFrom, uint64_t& nStakeModifier, int& nStakeModifierHeight, int64_t& nStakeModifierTime, bool fPrintProofOfStake);
bool ComputeNextStakeModifier(const CBlockIndex* pindexPrev, uint64_t& nStakeModifier, bool& fGeneratedStakeModifier);
bool ComputeStakeModifierV2(CBlockIndex* pindex, const uint256& kernel);

// Initialize the stake input object
bool initStakeInput(const CBlock& block, std::unique_ptr<CStakeInput>& stake, int nPreviousBlockHeight);

// Check kernel hash target and coinstake signature
// Sets hashProofOfStake on success return
bool CheckProofOfStake(const CBlock& block, uint256& hashProofOfStake, const CBlockIndex* pindex);
bool CheckStakeKernelHash(const CBlockIndex* pindexPrev, const unsigned int nBits, CStakeInput* stake, const unsigned int nTimeTx, uint256& hashProofOfStake, const bool fVerify = false);
// Returns the proof of stake hash
bool GetHashProofOfStake(const CBlockIndex* pindexPrev, CStakeInput* stake, const unsigned int nTimeTx, const bool fVerify, uint256& hashProofOfStakeRet);

bool HasStakeMinAgeOrDepth(const int contextHeight, const uint32_t contextTime,
        const int utxoFromBlockHeight, const uint32_t utxoFromBlockTime);

// Check Kernel Stake for pre DGW period
bool GetKernelStakeModifierPreDGW(uint256 hashBlockFrom, uint64_t& nStakeModifier, int& nStakeModifierHeight, int64_t& nStakeModifierTime, bool fPrintProofOfStake);

bool ContextualCheckZerocoinStake(int nPreviousBlockHeight, CStakeInput* stake);

int64_t GetTimeSlot(const int64_t nTime);

bool SetPOSParameters(const CBlock& block, CValidationState& state,CBlockIndex* pindexNew);

#endif // BITCOIN_KERNEL_H
