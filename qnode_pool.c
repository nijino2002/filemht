#include "qnode_pool.h"
#include <pthread.h>

// 静态池链表头（空闲QNode）
static PQNode g_qnode_pool_free_list = NULL;
static size_t g_qnode_pool_alloc_count = 0;
static pthread_mutex_t g_qnode_pool_lock = PTHREAD_MUTEX_INITIALIZER;

PQNode pool_alloc_qnode(void) {
    PQNode node = NULL;

    pthread_mutex_lock(&g_qnode_pool_lock);

    if (g_qnode_pool_free_list) {
        node = g_qnode_pool_free_list;
        g_qnode_pool_free_list = g_qnode_pool_free_list->next;
    } else {
        node = (PQNode)malloc(sizeof(QNode));
        if (node)
            g_qnode_pool_alloc_count++;
    }

    pthread_mutex_unlock(&g_qnode_pool_lock);

    if (!node) return NULL;

    // 初始化字段（在锁外完成）
    node->m_level = 0;
    node->m_is_written = FALSE;
    node->m_is_supplementary_node = 0;
    node->m_is_zero_node = 0;
    node->m_RMSTL_page_no = 0;
    node->m_MHTNode_ptr = NULL;
    node->prev = NULL;
    node->next = NULL;

    return node;
}

void pool_free_qnode(PQNode node) {
    if (!node) return;

    if (node->m_MHTNode_ptr) {
        deleteMHTNode(&(node->m_MHTNode_ptr));  // 释放 MHTNode
    }

    node->m_MHTNode_ptr = NULL;
    node->prev = NULL;

    pthread_mutex_lock(&g_qnode_pool_lock);

    // 入池头部
    node->next = g_qnode_pool_free_list;
    g_qnode_pool_free_list = node;

    pthread_mutex_unlock(&g_qnode_pool_lock);
}

void pool_cleanup_qnode(void) {
    pthread_mutex_lock(&g_qnode_pool_lock);

    PQNode tmp;
    while (g_qnode_pool_free_list) {
        tmp = g_qnode_pool_free_list;
        g_qnode_pool_free_list = g_qnode_pool_free_list->next;
        free(tmp);
    }
    g_qnode_pool_alloc_count = 0;

    pthread_mutex_unlock(&g_qnode_pool_lock);
}

size_t pool_qnode_allocated_count(void) {
    pthread_mutex_lock(&g_qnode_pool_lock);
    size_t count = g_qnode_pool_alloc_count;
    pthread_mutex_unlock(&g_qnode_pool_lock);
    return count;
}
