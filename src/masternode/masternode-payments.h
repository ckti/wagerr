// Copyright (c) 2014-2021 The Dash Core developers
// Copyright (c) 2020 The ION Core developers
// Copyright (c) 2021 The Wagerr developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_MASTERNODE_MASTERNODE_PAYMENTS_H
#define BITCOIN_MASTERNODE_MASTERNODE_PAYMENTS_H

#include <util.h>
#include <core_io.h>
#include <key.h>
#include <net_processing.h>
#include <utilstrencodings.h>

#include <evo/deterministicmns.h>

class CBlockReward;
class CMasternodePayments;

/// TODO: all 4 functions do not belong here really, they should be refactored/moved somewhere (main.cpp ?)
bool IsBlockValueValid(const CBlock& block, const int nBlockHeight, const CBlockReward& blockRewardIn, const CAmount coinstakeValueIn, std::string& strErrorRet);
bool IsBlockPayeeValid(const CTransactionRef txNewMiner, const CTransactionRef txNewStaker, int nBlockHeight, CBlockReward blockReward);
void FillBlockPayments(CMutableTransaction& txNew, int nBlockHeight, CBlockReward& blockReward, std::vector<CTxOut>& voutMasternodePaymentsRet, std::vector<CTxOut>& voutSuperblockPaymentsRet);

extern CMasternodePayments mnpayments;

//
// Masternode Payments Class
// Keeps track of who should get paid for which blocks
//

class CMasternodePayments
{
public:
    static bool GetBlockTxOuts(int nBlockHeight, CBlockReward& blockReward, std::vector<CTxOut>& voutMasternodePaymentsRet);
    static bool IsTransactionValid(const CTransaction& txNew, int nBlockHeight, CBlockReward blockReward);

    static bool GetMasternodeTxOuts(int nBlockHeight, CBlockReward& blockReward, std::vector<CTxOut>& voutMasternodePaymentsRet);
};

#endif // BITCOIN_MASTERNODE_MASTERNODE_PAYMENTS_H
