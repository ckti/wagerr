#!/usr/bin/env python3
# Copyright (c) 2017-2020 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test that the mempool ensures transaction delivery by periodically sending
to peers until a GETDATA is received."""

import time

from test_framework.mininode import P2PTxInvStore
from test_framework.test_framework import WagerrTestFramework
from test_framework.util import (
    assert_equal,
    connect_nodes,
    create_confirmed_utxos,
    disconnect_nodes,
)


class MempoolUnbroadcastTest(WagerrTestFramework):
    def set_test_params(self):
        self.num_nodes = 2

    def skip_test_if_missing_module(self):
        self.skip_if_no_wallet()

    def run_test(self):
        self.test_broadcast()
        self.test_txn_removal()

    def test_broadcast(self):
        self.log.info("Test that mempool reattempts delivery of locally submitted transaction")
        node = self.nodes[0]

        min_relay_fee = node.getnetworkinfo()["relayfee"]
        utxos = create_confirmed_utxos(min_relay_fee, node, 10)

        disconnect_nodes(node, 1)

        self.log.info("Generate transactions that only node 0 knows about")

        # generate a wallet txn
        addr = node.getnewaddress()
        wallet_tx_hsh = node.sendtoaddress(addr, 0.0001)

        # generate a txn using sendrawtransaction
        us0 = utxos.pop()
        inputs = [{"txid": us0["txid"], "vout": us0["vout"]}]
        outputs = {addr: 0.0001}
        tx = node.createrawtransaction(inputs, outputs)
        node.settxfee(min_relay_fee)
        txF = node.fundrawtransaction(tx)
        txFS = node.signrawtransactionwithwallet(txF["hex"])
        rpc_tx_hsh = node.sendrawtransaction(txFS["hex"])

        # check that second node doesn't have these two txns
        mempool = self.nodes[1].getrawmempool()
        assert rpc_tx_hsh not in mempool
        assert wallet_tx_hsh not in mempool

        # ensure that unbroadcast txs are persisted to mempool.dat
        self.restart_node(0)

        self.log.info("Reconnect nodes & check if they are sent to node 1")
        connect_nodes(node, 1)

        # fast forward into the future & ensure that the second node has the txns
        node.mockscheduler(15 * 60)  # 15 min in seconds
        self.sync_mempools(timeout=30)
        mempool = self.nodes[1].getrawmempool()
        assert rpc_tx_hsh in mempool
        assert wallet_tx_hsh in mempool

        self.log.info("Add another connection & ensure transactions aren't broadcast again")

        conn = node.add_p2p_connection(P2PTxInvStore())
        node.mockscheduler(15 * 60)
        time.sleep(5)
        assert_equal(len(conn.get_invs()), 0)

    def test_txn_removal(self):
        self.log.info("Test that transactions removed from mempool are removed from unbroadcast set")
        node = self.nodes[0]
        disconnect_nodes(node, 1)
        node.disconnect_p2ps

        # since the node doesn't have any connections, it will not receive
        # any GETDATAs & thus the transaction will remain in the unbroadcast set.
        addr = node.getnewaddress()
        txhsh = node.sendtoaddress(addr, 0.0001)

        # check transaction was removed from unbroadcast set due to presence in
        # a block
        removal_reason = "Removed {} from set of unbroadcast txns before confirmation that txn was sent out".format(txhsh)
        with node.assert_debug_log([removal_reason]):
            node.generate(1)

if __name__ == "__main__":
    MempoolUnbroadcastTest().main()
