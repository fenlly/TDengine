###################################################################
#           Copyright (c) 2016 by TAOS Technologies, Inc.
#                     All rights reserved.
#
#  This file is proprietary and confidential to TAOS Technologies.
#  No part of this file may be reproduced, stored, transmitted,
#  disclosed or used in any form or by any means other than as
#  expressly provided by the written permission from Jianhui Tao
#
###################################################################

# -*- coding: utf-8 -*-
import os
import frame
import frame.etool
from frame.log import *
from frame.cases import *
from frame.sql import *
from frame.caseBase import *
from frame import *


class TDTestCase(TBase):
    def caseDescription(self):
        """
        [TD-11510] taosBenchmark test cases
        """



    def run(self):
        binPath = etool.benchMarkFile()
        cmd = "%s -f ./tools/benchmark/basic/json/stmt_sample_use_ts-subtable.json" % binPath
        tdLog.info("%s" % cmd)
        os.system("%s" % cmd)
        tdSql.execute("reset query cache")
        tdSql.query("show db.tables")
        tdSql.checkRows(8)
        tdSql.query("select count(*) from db.stb")
        tdSql.checkData(0, 0, 32)
        tdSql.query("select * from db.stb0")
        tdSql.checkRows(4)
        tdSql.checkData(0, 1, 1)
        tdSql.checkData(1, 1, 2)
        tdSql.checkData(2, 1, 3)
        tdSql.checkData(3, 1, None)

        tdSql.query("select * from db.stb3")
        tdSql.checkRows(4)
        tdSql.checkData(0, 1, 300)
        tdSql.checkData(1, 1, 600)
        tdSql.checkData(2, 1, 900)
        tdSql.checkData(3, 1, None)

        tdSql.query("select * from db.stb5")
        tdSql.checkRows(4)
        tdSql.checkData(0, 1, 500)
        tdSql.checkData(1, 1, 1000)
        tdSql.checkData(2, 1, 1500)
        tdSql.checkData(3, 1, None)

        tdSql.query("select distinct(t0) from db.stb")
        tdSql.checkRows(2)

        dbresult = tdSql.res
        if dbresult[0][0] not in (17, None):
            tdLog.exit("result[0][0]: {}".format(dbresult[0][0]))
        else:
            tdLog.info("result[0][0]: {}".format(dbresult[0][0]))

    def stop(self):
        tdSql.close()
        tdLog.success("%s successfully executed" % __file__)


tdCases.addWindows(__file__, TDTestCase())
tdCases.addLinux(__file__, TDTestCase())
