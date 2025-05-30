/*
 * Copyright (c) 2019 TAOS Data, Inc. <jhtao@taosdata.com>
 *
 * This program is free software: you can use, redistribute, and/or modify
 * it under the terms of the GNU Affero General Public License, version 3
 * or later ("AGPL"), as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _TD_PLANNER_IMPL_H_
#define _TD_PLANNER_IMPL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "planner.h"
#include "tsimplehash.h"
#include "taoserror.h"


typedef struct SPhysiPlanContext {
  SPlanContext* pPlanCxt;
  int32_t       errCode;
  int16_t       nextDataBlockId;
  SArray*       pLocationHelper;
  SArray*       pProjIdxLocHelper;
  bool          hasScan;
  bool          hasSysScan;
} SPhysiPlanContext;

#define planFatal(param, ...)  qFatal ("plan " param, ##__VA_ARGS__)
#define planError(param, ...)  qError ("plan " param, ##__VA_ARGS__)
#define planWarn(param, ...)   qWarn  ("plan " param, ##__VA_ARGS__)
#define planInfo(param, ...)   qInfo  ("plan " param, ##__VA_ARGS__)
#define planDebug(param, ...)  qDebug ("plan " param, ##__VA_ARGS__)
#define planDebugL(param, ...) qDebugL("plan " param, ##__VA_ARGS__)
#define planTrace(param, ...)  qTrace ("plan " param, ##__VA_ARGS__)

#define PLAN_ERR_RET(c)                \
  do {                                 \
    int32_t _code = c;                 \
    if (_code != TSDB_CODE_SUCCESS) {  \
      terrno = _code;                  \
      return _code;                    \
    }                                  \
  } while (0)
#define PLAN_RET(c)                    \
  do {                                 \
    int32_t _code = c;                 \
    if (_code != TSDB_CODE_SUCCESS) {  \
      terrno = _code;                  \
    }                                  \
    return _code;                      \
  } while (0)
#define PLAN_ERR_JRET(c)              \
  do {                                \
    code = c;                         \
    if (code != TSDB_CODE_SUCCESS) {  \
      terrno = code;                  \
      goto _return;                   \
    }                                 \
  } while (0)


int32_t generateUsageErrMsg(char* pBuf, int32_t len, int32_t errCode, ...);
int32_t createColumnByRewriteExprs(SNodeList* pExprs, SNodeList** pList);
int32_t createColumnByRewriteExpr(SNode* pExpr, SNodeList** pList);
int32_t replaceLogicNode(SLogicSubplan* pSubplan, SLogicNode* pOld, SLogicNode* pNew);
int32_t adjustLogicNodeDataRequirement(SLogicNode* pNode, EDataOrderLevel requirement);

int32_t createLogicPlan(SPlanContext* pCxt, SLogicSubplan** pLogicSubplan);
int32_t optimizeLogicPlan(SPlanContext* pCxt, SLogicSubplan* pLogicSubplan);
int32_t splitLogicPlan(SPlanContext* pCxt, SLogicSubplan* pLogicSubplan);
int32_t scaleOutLogicPlan(SPlanContext* pCxt, SLogicSubplan* pLogicSubplan, SQueryLogicPlan** pLogicPlan);
int32_t createPhysiPlan(SPlanContext* pCxt, SQueryLogicPlan* pLogicPlan, SQueryPlan** pPlan, SArray* pExecNodeList);
int32_t validateQueryPlan(SPlanContext* pCxt, SQueryPlan* pPlan);

bool        getBatchScanOptionFromHint(SNodeList* pList);
bool        getSortForGroupOptHint(SNodeList* pList);
bool        getParaTablesSortOptHint(SNodeList* pList);
bool        getSmallDataTsSortOptHint(SNodeList* pList);
bool        getHashJoinOptHint(SNodeList* pList);
bool        getOptHint(SNodeList* pList, EHintOption hint);
SLogicNode* getLogicNodeRootNode(SLogicNode* pCurr);
int32_t     collectTableAliasFromNodes(SNode* pNode, SSHashObj** ppRes);
bool        isPartTableAgg(SAggLogicNode* pAgg);
bool        isPartTagAgg(SAggLogicNode* pAgg);
bool        isPartTableWinodw(SWindowLogicNode* pWindow);
bool        keysHasCol(SNodeList* pKeys);
bool        keysHasTbname(SNodeList* pKeys);
bool        projectCouldMergeUnsortDataBlock(SProjectLogicNode* pProject);
SFunctionNode* createGroupKeyAggFunc(SColumnNode* pGroupCol);
int32_t getTimeRangeFromNode(SNode** pPrimaryKeyCond, STimeWindow* pTimeRange, bool* pIsStrict);
int32_t tagScanSetExecutionMode(SScanLogicNode* pScan);

#define CLONE_LIMIT 1
#define CLONE_SLIMIT 1 << 1
#define CLONE_LIMIT_SLIMIT (CLONE_LIMIT | CLONE_SLIMIT)
int32_t cloneLimit(SLogicNode* pParent, SLogicNode* pChild, uint8_t cloneWhat, bool* pCloned);
int32_t sortPriKeyOptGetSequencingNodesImpl(SLogicNode* pNode, bool groupSort, SSortLogicNode* pSort,
                                                   bool* pNotOptimize, SNodeList** pSequencingNodes, bool* keepSort);
bool isColRefExpr(const SColumnNode* pCol, const SExprNode* pExpr);
void rewriteTargetsWithResId(SNodeList* pTargets);


#ifdef __cplusplus
}
#endif

#endif /*_TD_PLANNER_IMPL_H_*/
