// Copyright (c) 2017-2022 The Dash Core developers
// Copyright (c) 2021 The Wagerr developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_EVO_CBTX_H
#define BITCOIN_EVO_CBTX_H

#include <primitives/transaction.h>
#include <pos/rewards.h>
#include <univalue.h>

class CBlock;
class CBlockIndex;
class CBlockReward;
class CCoinsViewCache;
class CValidationState;

// proof-of-stake specific fields
enum {
    CSTX_POS  = (1 << 0),  // PoS, not PoW
    CSTX_SPLIT_COINSTAKE  = (1 << 1),  // two coinstake outputs instead of one
    CSTX_MASTERNODE_OUTPUT = (1 << 3), // has masternode reward output
    CSTX_OPERATOR_OUTPUT = (1 << 4), // has masternode reward output

    CSTX_MAX = (1 << 5),
};

namespace llmq {
class CQuorumBlockProcessor;
}// namespace llmq

// coinbase transaction
class CCbTx
{
public:
    static constexpr auto SPECIALTX_TYPE = TRANSACTION_COINBASE;
    static constexpr uint16_t CURRENT_VERSION = 2;

    uint16_t nVersion{CURRENT_VERSION};
    int32_t nHeight{0};
    uint256 merkleRootMNList;
    uint256 merkleRootQuorums;
    uint8_t coinstakeFlags{0};

    SERIALIZE_METHODS(CCbTx, obj)
    {
        READWRITE(obj.nVersion, obj.nHeight, obj.merkleRootMNList, obj.coinstakeFlags);

        if (obj.nVersion >= 2) {
            READWRITE(obj.merkleRootQuorums);
        }
    }

    std::string ToString() const;

    void ToJson(UniValue& obj) const
    {
        obj.clear();
        obj.setObject();
        obj.pushKV("version", (int)nVersion);
        obj.pushKV("height", nHeight);
        obj.pushKV("coinstakeFlags", (int)coinstakeFlags);
        obj.pushKV("merkleRootMNList", merkleRootMNList.ToString());
        if (nVersion >= 2) {
            obj.pushKV("merkleRootQuorums", merkleRootQuorums.ToString());
        }
    }
};

bool CheckCbTx(const CTransaction& tx, const CBlockIndex* pindexPrev, CValidationState& state);

bool CheckCbTxMerkleRoots(const CBlock& block, const CBlockIndex* pindex, const llmq::CQuorumBlockProcessor& quorum_block_processor, CValidationState& state, const CCoinsViewCache& view);
bool CheckCbTxCoinstakeFlags(const CCbTx& cbTx, const CBlock& block, CValidationState& state);
void GetCbTxCoinstakeFlags(const uint8_t nCoinstakeFlags, bool& fPos, bool& fSplitCoinstake, bool& fMasternodeTx, bool& fOperatorTx);
void CalcCbTxCoinstakeFlags(uint8_t& nCoinstakeFlags, const bool fSplit, const bool fSplitCoinstake, const bool fMasternodeTx, const bool fOperatorTx);
void CalcCbTxCoinstakeFlags(uint8_t& nCoinstakeFlags, const CBlockReward blockReward);
bool CheckCoinstakeOutputs(const CBlock& block, const bool fPos, const bool fSplitCoinstake, const bool fMasternodeTx, const bool fOperatorTx);
bool CalcCbTxMerkleRootMNList(const CBlock& block, const CBlockIndex* pindexPrev, uint256& merkleRootRet, CValidationState& state, const CCoinsViewCache& view);
bool CalcCbTxMerkleRootQuorums(const CBlock& block, const CBlockIndex* pindexPrev, const llmq::CQuorumBlockProcessor& quorum_block_processor, uint256& merkleRootRet, CValidationState& state);

#endif // BITCOIN_EVO_CBTX_H
