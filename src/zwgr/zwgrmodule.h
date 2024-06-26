// Copyright (c) 2019 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//
#ifndef PIVX_ZWGRMODULE_H
#define PIVX_ZWGRMODULE_H

#include "libzerocoin/bignum.h"
#include "libzerocoin/Denominations.h"
#include "libzerocoin/CoinSpend.h"
#include "libzerocoin/Coin.h"
#include "libzerocoin/SpendType.h"
#include "primitives/transaction.h"
#include "script/script.h"
#include "serialize.h"
#include "uint256.h"
#include <streams.h>
#include <util/strencodings.h>
#include "zwgr/zerocoin.h"
#include "chainparams.h"

static int const COIN_SPEND_PUBLIC_SPEND_VERSION = 3;

class PublicCoinSpend : public libzerocoin::CoinSpend{
public:

    PublicCoinSpend(libzerocoin::ZerocoinParams* params):pubCoin(params){};

    PublicCoinSpend(libzerocoin::ZerocoinParams* params,
            CBigNum serial, CBigNum randomness, CPubKey pubkey):pubCoin(params){
        this->coinSerialNumber = serial;
        this->randomness = randomness;
        this->pubkey = pubkey;
        this->spendType = libzerocoin::SpendType::SPEND;
        this->version = COIN_SPEND_PUBLIC_SPEND_VERSION;
    };

    ~PublicCoinSpend(){};

    template <typename Stream>
    PublicCoinSpend(
            libzerocoin::ZerocoinParams* params,
            Stream& strm):pubCoin(params){
        strm >> *this;
        this->spendType = libzerocoin::SpendType::SPEND;
    }

    const uint256 signatureHash() const override;
    void setVchSig(std::vector<unsigned char> vchSig) { this->vchSig = vchSig; };
    bool Verify(const libzerocoin::Accumulator& a, bool verifyParams = true) const override;
    bool validate() const;

    // Members
    CBigNum randomness;
    // prev out values
    uint256 txHash = ArithToUint256(0);
    unsigned int outputIndex = -1;
    libzerocoin::PublicCoin pubCoin;

    SERIALIZE_METHODS(PublicCoinSpend, obj)
    {
        READWRITE(obj.version);
        READWRITE(obj.coinSerialNumber);
        READWRITE(obj.randomness);
        READWRITE(obj.pubkey);
        READWRITE(obj.vchSig);
    }
};


class CValidationState;

namespace ZWGRModule {
//    bool createInput(CTxIn &in, CZerocoinMint& mint, uint256 hashTxOut);
//    PublicCoinSpend parseCoinSpend(const CTxIn &in);
    bool parseCoinSpend(const CTxIn &in, const CTransaction& tx, const CTxOut &prevOut, PublicCoinSpend& publicCoinSpend);
    bool validateInput(const CTxIn &in, const CTxOut &prevOut, const CTransaction& tx, PublicCoinSpend& ret);

    // Public zc spend parse
    /**
     *
     * @param in --> public zc spend input
     * @param tx --> input parent
     * @param publicCoinSpend ---> return the publicCoinSpend parsed
     * @return true if everything went ok
     */
    bool ParseZerocoinPublicSpend(const CTxIn &in, const CTransaction& tx, CValidationState& state, PublicCoinSpend& publicCoinSpend);
};


#endif //PIVX_ZWGRMODULE_H
