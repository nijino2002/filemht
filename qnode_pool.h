#ifndef QNODE_POOL_H
#define QNODE_POOL_H

#include "defs.h"
#include "dbqueue.h"  // 提供 PQNode 的定义
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

extern PQNode g_qnode_pool_free_list;
extern size_t g_qnode_pool_alloc_count;
extern pthread_mutex_t g_qnode_pool_lock;

/**
 * @brief 初始化 QNode 对象池。
 * 
 * @param prealloc_count: pre-allocated node number. 0 is accepted.
 * 本函数应在程序开始时调用一次，用于初始化对象池的状态。
 */
void init_qnode_pool(size_t prealloc_count);

/**
 * @brief 销毁 QNode 对象池并释放所有资源。
 * 
 * 本函数应在程序结束时调用一次，释放对象池中缓存的所有 QNode，
 * 并销毁相关的互斥锁资源。
 */
void destroy_qnode_pool(void);

/**
 * @brief 从对象池中分配一个 QNode 实例（线程安全）。
 *
 * @return PQNode 若成功则返回指针，否则返回 NULL。
 */
PQNode pool_alloc_qnode(void);

/**
 * @brief 将一个 QNode 回收到对象池（线程安全）。
 *
 * @param node 要回收的 QNode 指针，允许为 NULL。
 */
void pool_free_qnode(PQNode node);

/**
 * @brief 释放对象池中所有空闲 QNode（线程安全）。
 *
 * 应在程序结束或不再需要池时调用，用于清理资源。
 */
void pool_cleanup_qnode(void);

/**
 * @brief 获取当前通过池分配的 QNode 总数（线程安全）。
 *
 * @return size_t 分配计数（包括仍在使用和空闲链中的节点）。
 */
size_t pool_qnode_allocated_count(void);

#ifdef __cplusplus
}
#endif

#endif // QNODE_POOL_H
