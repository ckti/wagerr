#!/usr/bin/env python3
# Copyright (c) 2015-2016 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test transaction signing using the signrawtransaction* RPCs."""

from test_framework.test_framework import WagerrTestFramework
from test_framework.util import assert_equal, assert_raises_rpc_error


class SignRawTransactionsTest(WagerrTestFramework):
    def set_test_params(self):
        self.setup_clean_chain = True
        self.num_nodes = 1
        self.mn_count = 0
        self.fast_dip3_enforcement = False
        self.extra_args = [["-debug"]]

    def skip_test_if_missing_module(self):
        self.skip_if_no_wallet()

    def successful_signing_test(self):
        """Create and sign a valid raw transaction with one input.

        Expected results:

        1) The transaction has a complete set of signatures
        2) No script verification error occurred"""

        privKeys = ['THTeyaP8QLTG8zwG1AdYrnWqCaaAjbj7TcW9xRhJ7n6LRLCeg6Bc', 'TCiM4JqGNtShSZfSop7CEhSQzWNXXoony1yumWvW5gN5imKYp47E']

        inputs = [
            # Valid pay-to-pubkey scripts
            {'txid': 'a78e679f751b4f6b51e6691d437b198bef9a089c155c9d114d272093f9da23c2', 'vout': 0,
             'scriptPubKey': '76a91472c47e2427496e2f9103ca98553a1cdbc6b7d5e688ac'},
            {'txid': '6e11d884bba753bc6abc27f2d1aa8d74c7d6528928927160ee8cc003997ec67d', 'vout': 0,
             'scriptPubKey': '76a91472c47e2427496e2f9103ca98553a1cdbc6b7d5e688ac'},
        ]

        outputs = {'TLS3RUwUvDoFhkT8yScNJWzqX1eHBTRdH6': 0.1}

        rawTx = self.nodes[0].createrawtransaction(inputs, outputs)
        rawTxSigned = self.nodes[0].signrawtransactionwithkey(rawTx, privKeys, inputs)

        # 1) The transaction has a complete set of signatures
        assert rawTxSigned['complete']

        # 2) No script verification error occurred
        assert 'errors' not in rawTxSigned

    def test_with_lock_outputs(self):
        """Test correct error reporting when trying to sign a locked output"""
        self.nodes[0].encryptwallet("password")

        rawTx = '020000000156b958f78e3f24e0b2f4e4db1255426b0902027cb37e3ddadb52e37c3557dddb0000000000ffffffff01c0a6b929010000001600149a2ee8c77140a053f36018ac8124a6ececc1668a00000000'

        assert_raises_rpc_error(-13, "Please enter the wallet passphrase with walletpassphrase first", self.nodes[0].signrawtransactionwithwallet, rawTx)

    def script_verification_error_test(self):
        """Create and sign a raw transaction with valid (vin 0), invalid (vin 1) and one missing (vin 2) input script.

        Expected results:

        3) The transaction has no complete set of signatures
        4) Two script verification errors occurred
        5) Script verification errors have certain properties ("txid", "vout", "scriptSig", "sequence", "error")
        6) The verification errors refer to the invalid (vin 1) and missing input (vin 2)"""
        privKeys = ['THTeyaP8QLTG8zwG1AdYrnWqCaaAjbj7TcW9xRhJ7n6LRLCeg6Bc']

        inputs = [
            # Valid pay-to-pubkey script
            {'txid': 'a78e679f751b4f6b51e6691d437b198bef9a089c155c9d114d272093f9da23c2', 'vout': 0},
            # Invalid script
            {'txid': 'a78e679f751b4f6b51e6691d437b198bef9a089c155c9d114d272093f9da23c2', 'vout': 7},
            # Missing scriptPubKey
            {'txid': '9b907ef1e3c26fc71fe4a4b3580bc75264112f95050014157059c736f0202e71', 'vout': 1},
        ]

        scripts = [
            # Valid pay-to-pubkey script
            {'txid': 'a78e679f751b4f6b51e6691d437b198bef9a089c155c9d114d272093f9da23c2', 'vout': 0,
             'scriptPubKey': '76a91472c47e2427496e2f9103ca98553a1cdbc6b7d5e688ac'},
            # Invalid script
            {'txid': '5b8673686910442c644b1f4993d8f7753c7c8fcb5c87ee40d56eaeef25204547', 'vout': 7,
             'scriptPubKey': 'badbadbadbad'}
        ]

        outputs = {'TLS3RUwUvDoFhkT8yScNJWzqX1eHBTRdH6': 0.1}

        rawTx = self.nodes[0].createrawtransaction(inputs, outputs)

        # Make sure decoderawtransaction is at least marginally sane
        decodedRawTx = self.nodes[0].decoderawtransaction(rawTx)
        for i, inp in enumerate(inputs):
            assert_equal(decodedRawTx["vin"][i]["txid"], inp["txid"])
            assert_equal(decodedRawTx["vin"][i]["vout"], inp["vout"])

        # Make sure decoderawtransaction throws if there is extra data
        assert_raises_rpc_error(-22, "TX decode failed", self.nodes[0].decoderawtransaction, rawTx + "00")

        rawTxSigned = self.nodes[0].signrawtransactionwithkey(rawTx, privKeys, scripts)

        # 3) The transaction has no complete set of signatures
        assert not rawTxSigned['complete']

        # 4) Two script verification errors occurred
        assert 'errors' in rawTxSigned
        assert_equal(len(rawTxSigned['errors']), 3)

        # 5) Script verification errors have certain properties
        assert 'txid' in rawTxSigned['errors'][0]
        assert 'vout' in rawTxSigned['errors'][0]
        assert 'scriptSig' in rawTxSigned['errors'][0]
        assert 'sequence' in rawTxSigned['errors'][0]
        assert 'error' in rawTxSigned['errors'][0]

        # 6) The verification errors refer to the invalid (vin 1) and missing input (vin 2)
        assert_equal(rawTxSigned['errors'][0]['txid'], inputs[1]['txid'])
        assert_equal(rawTxSigned['errors'][0]['vout'], inputs[1]['vout'])
        assert_equal(rawTxSigned['errors'][1]['txid'], inputs[2]['txid'])
        assert_equal(rawTxSigned['errors'][1]['vout'], inputs[2]['vout'])

    def run_test(self):
        self.successful_signing_test()
        self.script_verification_error_test()
        self.test_with_lock_outputs()


if __name__ == '__main__':
    SignRawTransactionsTest().main()
