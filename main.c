#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DATA_COUNT 100
#define SEARCH_COUNT 50
#define NUM_TRIALS 20
#define VALUE_RANGE 1001

typedef struct Node {
    int val;
    struct Node* left;
    struct Node* right;
    int height; 
} Node;

Node* make_node(int v) {
    Node* n = (Node*)malloc(sizeof(Node));
    if (n) {
        n->val = v;
        n->left = n->right = NULL;
        n->height = 1;
    }
    return n;
}

int node_h(Node* n) {
    return n ? n->height : 0;
}

void sync_h(Node* n) {
    if (!n) return;
    int l = node_h(n->left);
    int r = node_h(n->right);
    n->height = (l > r ? l : r) + 1;
}


Node* shift_right(Node* parent) {
    Node* child = parent->left;
    Node* sub = child->right;

    child->right = parent;
    parent->left = sub;

    sync_h(parent);
    sync_h(child);

    return child;
}


Node* shift_left(Node* parent) {
    Node* child = parent->right;
    Node* sub = child->left;

    child->left = parent;
    parent->right = sub;

    sync_h(parent);
    sync_h(child);

    return child;
}

// 1. 배열 삽입
int add_to_array(int arr[], int* len, int v, long long* comps) {
    for (int i = 0; i < *len; i++) {
        (*comps)++;
        if (arr[i] == v) return 0; // 중복
    }
    arr[(*len)++] = v;
    return 1;
}

// 2. BST 삽입
Node* add_to_bst(Node* root, int v, long long* comps) {
    if (!root) return make_node(v);

    (*comps)++;
    if (v < root->val) {
        root->left = add_to_bst(root->left, v, comps);
    } else if (v > root->val) {
        root->right = add_to_bst(root->right, v, comps);
    }
    return root; // 중복이면 아무 작업 없이 리턴
}

// 3. AVL 삽입
Node* add_to_avl(Node* root, int v, long long* comps) {
    if (!root) return make_node(v);

    (*comps)++;
    if (v < root->val) {
        root->left = add_to_avl(root->left, v, comps);
    } else if (v > root->val) {
        root->right = add_to_avl(root->right, v, comps);
    } else {
        return root; // 중복 발생 시 회전 연산 스킵
    }

    sync_h(root);
    int balance = node_h(root->left) - node_h(root->right);

   
    if (balance > 1) {
        if (v < root->left->val) {
            return shift_right(root); // LL
        } else {
            root->left = shift_left(root->left); // LR
            return shift_right(root);
        }
    }
   
    if (balance < -1) {
        if (v > root->right->val) {
            return shift_left(root); // RR
        } else {
            root->right = shift_right(root->right); // RL
            return shift_left(root);
        }
    }

    return root;
}

//  $O(N)$ 재귀 필요
int calc_bst_depth(Node* root) {
    if (!root) return 0;
    int l = calc_bst_depth(root->left);
    int r = calc_bst_depth(root->right);
    return (l > r ? l : r) + 1;
}

int find_in_tree(Node* root, int target, int* comps) {
    *comps = 0;
    Node* cur = root;
    while (cur) {
        (*comps)++;
        if (target == cur->val) return 1;
        cur = (target < cur->val) ? cur->left : cur->right;
    }
    return 0;
}

int find_in_array(int arr[], int len, int target, int* comps) {
    *comps = 0;
    for (int i = 0; i < len; i++) {
        (*comps)++;
        if (arr[i] == target) return 1;
    }
    return 0;
}

void clear_tree(Node* root) {
    if (!root) return;
    clear_tree(root->left);
    clear_tree(root->right);
    free(root);
}


void run_random_experiment() {
    printf("====================================================\n");
    printf(" [실험 1] 난수 데이터 %d회 반복 실험 (평균값)\n", NUM_TRIALS);
    printf("====================================================\n");

    long long sum_arr_build = 0, sum_bst_build = 0, sum_avl_build = 0;
    long long sum_arr_search = 0, sum_bst_search = 0, sum_avl_search = 0;
    int sum_bst_h = 0, sum_avl_h = 0;

    for (int t = 0; t < NUM_TRIALS; t++) {
        int arr[DATA_COUNT];
        int arr_len = 0;
        Node *bst = NULL, *avl = NULL;

        long long ab = 0, bb = 0, avb = 0;

    
        for (int i = 0; i < DATA_COUNT; i++) {
            int val = rand() % VALUE_RANGE;
            add_to_array(arr, &arr_len, val, &ab);
            bst = add_to_bst(bst, val, &bb);
            avl = add_to_avl(avl, val, &avb);
        }

        sum_arr_build += ab;
        sum_bst_build += bb;
        sum_avl_build += avb;

        sum_bst_h += calc_bst_depth(bst);
        sum_avl_h += node_h(avl);

        long long as = 0, bs = 0, avs = 0;
        for (int i = 0; i < SEARCH_COUNT; i++) {
            int target = rand() % VALUE_RANGE;
            int c1 = 0, c2 = 0, c3 = 0;

            find_in_array(arr, arr_len, target, &c1);
            find_in_tree(bst, target, &c2);
            find_in_tree(avl, target, &c3);

            as += c1; bs += c2; avs += c3;
        }

        sum_arr_search += as;
        sum_bst_search += bs;
        sum_avl_search += avs;

        clear_tree(bst);
        clear_tree(avl);
    }

    printf("1. 트리의 평균 높이 / 배열 길이\n");
    printf("   - 배열 길이 : %d\n", DATA_COUNT);
    printf("   - BST 높이  : %.2f\n", (double)sum_bst_h / NUM_TRIALS);
    printf("   - AVL 높이  : %.2f\n\n", (double)sum_avl_h / NUM_TRIALS);

    printf("2. 자료구조 구축 시 평균 비교 횟수 (%d개 데이터)\n", DATA_COUNT);
    printf("   - Array : %.2f 번\n", (double)sum_arr_build / NUM_TRIALS);
    printf("   - BST   : %.2f 번\n", (double)sum_bst_build / NUM_TRIALS);
    printf("   - AVL   : %.2f 번\n\n", (double)sum_avl_build / NUM_TRIALS);

    printf("3. 평균 탐색 비교 횟수 (%d개 키 탐색 기준)\n", SEARCH_COUNT);
    printf("   - Array 평균 : %.2f 번\n", (double)sum_arr_search / (NUM_TRIALS * SEARCH_COUNT));
    printf("   - BST 평균   : %.2f 번\n", (double)sum_bst_search / (NUM_TRIALS * SEARCH_COUNT));
    printf("   - AVL 평균   : %.2f 번\n\n", (double)sum_avl_search / (NUM_TRIALS * SEARCH_COUNT));
}

void run_sorted_experiment() {
    printf("====================================================\n");
    printf(" [실험 2] 정렬된 데이터 삽입 (1 ~ 100) - 최악의 케이스\n");
    printf("====================================================\n");

    int arr[DATA_COUNT];
    int arr_len = 0;
    Node *bst = NULL, *avl = NULL;

    long long ab = 0, bb = 0, avb = 0;

    for (int i = 1; i <= DATA_COUNT; i++) {
        add_to_array(arr, &arr_len, i, &ab);
        bst = add_to_bst(bst, i, &bb);
        avl = add_to_avl(avl, i, &avb);
    }

    printf("1. 트리 높이 비교\n");
    printf("   - BST 높이 : %d (경사 트리 발생! O(N)으로 전락)\n", calc_bst_depth(bst));
    printf("   - AVL 높이 : %d (회전을 통해 O(log N) 높이 유지)\n\n", node_h(avl));

    printf("2. 구축 비교 횟수\n");
    printf("   - Array : %lld 번\n", ab);
    printf("   - BST   : %lld 번 (1+2+...+99 연산)\n", bb);
    printf("   - AVL   : %lld 번\n\n", avb);

    // 탐색 (마지막 값 100 탐색)
    int c1 = 0, c2 = 0, c3 = 0;
    find_in_array(arr, arr_len, 100, &c1);
    find_in_tree(bst, 100, &c2);
    find_in_tree(avl, 100, &c3);

    printf("3. '100' 탐색 시 비교 횟수\n");
    printf("   - Array : %d 번\n", c1);
    printf("   - BST   : %d 번 (일자 리스트 탐색과 동일)\n", c2);
    printf("   - AVL   : %d 번 (트리 높이만큼만 탐색)\n\n", c3);

    clear_tree(bst);
    clear_tree(avl);
}

int main() {
    srand((unsigned int)time(NULL));

    run_random_experiment();
    run_sorted_experiment();

    return 0;
}
