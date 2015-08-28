#pragma once
#include "stdafx.h"
#include <vector>
#include <algorithm>
#include <set>

template <class T>
struct Node
{
    int Row, Col;
    T Value;
    Node *Right, *Down;

    Node();
    Node(int row, int col);
    Node(int row, int col, T val);
};

template <class T>
class OrthogonalList
{
public:
    OrthogonalList();
    OrthogonalList(int width, int height);
    OrthogonalList(const OrthogonalList<T> &other);
    ~OrthogonalList();

    int GetHeight() const;
    int GetWidth() const;

    Node<T> &SeekRight(int row, int col);
    Node<T> &SeekRight(Node<T> &nr, int col);
    Node<T> &SeekDown(int row, int col);
    Node<T> &SeekDown(int row, Node<T> &nc);

    Node<T> &GetRowHead(int row);
    const Node<T> &GetRowHead(int row) const;
    Node<T> &GetColHead(int col);
    const Node<T> &GetColHead(int col) const;

    Node<T> &Add(int row, int col, T val);
    Node<T> &Add(Node<T> &nr, Node<T> &nc, T val);

    void Remove(int row, int col);
    void Remove(Node<T> &nr, Node<T> &nc);
    void Remove(const Node<T> &node);

    void RemoveRow(int row);
    void RemoveCol(int col);

    void InsertRow(int row);
    void InsertCol(int col);

    void ExtendHeight(int rows);
    void ExtendWidth(int cols);
private:
    std::vector<Node<T>> m_Rows, m_Cols;
};

template <class T>
Node<T>::Node() : Row(-1), Col(-1), Right(nullptr), Down(nullptr) {}

template <class T>
Node<T>::Node(int row, int col) : Row(row), Col(col), Right(nullptr), Down(nullptr) {}

template <class T>
Node<T>::Node(int row, int col, T val) : Row(row), Col(col), Value(val), Right(nullptr), Down(nullptr) {}

template <class T>
OrthogonalList<T>::OrthogonalList() {}

template <class T>
OrthogonalList<T>::OrthogonalList(int width, int height)
{
    ExtendHeight(height);
    ExtendWidth(width);
}

template <class T>
OrthogonalList<T>::OrthogonalList(const OrthogonalList<T> &other)
{
    ExtendHeight(other.GetHeight());
    ExtendWidth(other.GetWidth());

    for (auto i = 0; i < m_Rows.size(); i++)
    {
        const Node<T> *n = other.GetRowHead(i).Right;
        auto nr = &m_Rows[i];
        while (n != nullptr)
        {
            nr = &Add(*nr, m_Cols[n->Col], n->Value);
            n = n->Right;
        }
    }
}

template <class T>
OrthogonalList<T>::~OrthogonalList()
{
    for (auto &node : m_Rows)
        while (node.Right != nullptr)
            Remove(*node.Right);
}

template <class T>
int OrthogonalList<T>::GetHeight() const
{
    return m_Rows.size();
}

template <class T>
int OrthogonalList<T>::GetWidth() const
{
    return m_Cols.size();
}

template <class T>
Node<T> &OrthogonalList<T>::SeekRight(int row, int col)
{
    return SeekRight(m_Rows[row], col);
}

template <class T>
Node<T> &OrthogonalList<T>::SeekRight(Node<T> &nr, int col)
{
    auto ptr = &nr;
    while (true)
    {
        if (ptr->Right == nullptr || ptr->Right->Col >= col)
            break;
        ptr = ptr->Right;
    }
    return *ptr;
}

template <class T>
Node<T> &OrthogonalList<T>::SeekDown(int row, int col)
{
    return SeekDown(row, m_Cols[col]);
}

template <class T>
Node<T> &OrthogonalList<T>::SeekDown(int row, Node<T> &nc)
{
    auto ptr = &nc;
    while (true)
    {
        if (ptr->Down == nullptr || ptr->Down->Row >= row)
            break;
        ptr = ptr->Down;
    }
    return *ptr;
}

template <class T>
Node<T> &OrthogonalList<T>::GetRowHead(int row)
{
    return m_Rows[row];
}

template <class T>
const Node<T> &OrthogonalList<T>::GetRowHead(int row) const
{
    return m_Rows[row];
}

template <class T>
Node<T> &OrthogonalList<T>::GetColHead(int col)
{
    return m_Cols[col];
}

template <class T>
const Node<T> &OrthogonalList<T>::GetColHead(int col) const
{
    return m_Cols[col];
}

template <class T>
Node<T> &OrthogonalList<T>::Add(int row, int col, T val)
{
    return Add(GetRowHead(row), GetColHead(col), val);
}

template <class T>
Node<T> &OrthogonalList<T>::Add(Node<T> &nr, Node<T> &nc, T val)
{
    auto ptrR = &SeekRight(nr, nc.Col);
    if (ptrR->Down != nullptr && ptrR->Down->Row == ptrR->Row)
    {
        ptrR->Down->Value = val;
        return *ptrR->Down;
    }
    auto ptrC = &SeekDown(ptrR->Row, nc);

    Node<T> *node = new Node<T>(ptrR->Row, ptrC->Col, val);
    node->Right = ptrR->Right;
    node->Down = ptrC->Down;
    ptrR->Right = node;
    ptrC->Down = node;

    return *node;
}

template <class T>
void OrthogonalList<T>::Remove(int row, int col)
{
    Remove(m_Rows[row], m_Cols[col]);
}

template <class T>
void OrthogonalList<T>::Remove(Node<T> &nr, Node<T> &nc)
{
    auto left = &SeekRight(nr, nc.Col);
    auto up = &SeekDown(nr.Row, nc);

    if (left->Right == nullptr || up->Down == nullptr)
        return;

    auto node = left->Right;
    if (node != up->Down)
        return;
    if (node->Col != nc.Col)
        throw;

    left->Right = node->Right;
    up->Down = node->Down;

    delete node;
}

template <class T>
void OrthogonalList<T>::Remove(const Node<T> &node)
{
    Remove(node.Row, node.Col);
}

template <class T>
void OrthogonalList<T>::RemoveRow(int row)
{
    while (m_Rows[row].Right != nullptr)
        Remove(row, m_Rows[row].Right->Col);
    m_Rows.erase(m_Rows.begin() + row);
    for (auto i = row; i < m_Rows.size(); ++i)
    {
        auto n = &m_Rows[i];
        while (n != nullptr)
        {
            --n->Row;
            n = n->Right;
        }
    }
}

template <class T>
void OrthogonalList<T>::RemoveCol(int col)
{
    while (m_Cols[col].Down != nullptr)
        Remove(m_Cols[col].Down->Row, col);
    m_Cols.erase(m_Cols.begin() + col);
    for (auto i = col; i < m_Cols.size(); ++i)
    {
        auto n = &m_Cols[i];
        while (n != nullptr)
        {
            --n->Col;
            n = n->Down;
        }
    }
}

template <class T>
void OrthogonalList<T>::InsertRow(int row)
{
    for (auto i = row; i < m_Rows.size(); ++i)
    {
        auto n = &m_Rows[i];
        while (n != nullptr)
        {
            ++n->Row;
            n = n->Right;
        }
    }
    m_Rows.insert(m_Rows.begin() + row, Node<T>(row, -1));
}

template <class T>
void OrthogonalList<T>::InsertCol(int col)
{
    for (auto i = col; i < m_Cols.size(); ++i)
    {
        auto n = &m_Cols[i];
        while (n != nullptr)
        {
            ++n->Col;
            n = n->Down;
        }
    }
    m_Cols.insert(m_Cols.begin() + col, Node<T>(-1, col));
}

template <class T>
void OrthogonalList<T>::ExtendHeight(int rows)
{
    if (m_Rows.size() >= rows)
        return;

    auto size = m_Rows.size();

    m_Rows.reserve(rows);

    for (auto i = size; i < rows; ++i)
        m_Rows.emplace_back(i, -1);
}

template <class T>
void OrthogonalList<T>::ExtendWidth(int cols)
{
    if (m_Cols.size() >= cols)
        return;

    auto size = m_Cols.size();

    m_Cols.reserve(cols);

    for (auto i = size; i < cols; ++i)
        m_Cols.emplace_back(-1, i);
}

template <class T>
bool Check(const OrthogonalList<T> &ol)
{
    std::set<std::pair<int, int>> pairs;
    for (auto i = 0; i < ol.GetHeight(); ++i)
    {
        auto n = &ol.GetRowHead(i);
        while (n != nullptr)
        {
            if (n->Row != i)
                return false;
            if (n->Col >= 0)
                if (pairs.emplace(n->Row, n->Col).second == false)
                    return false;
            n = n->Right;
        }
    }
    for (auto i = 0; i < ol.GetWidth(); ++i)
    {
        auto n = &ol.GetColHead(i);
        while (n != nullptr)
        {
            if (n->Col != i)
                return false;
            if (n->Row >= 0)
            {
                auto v = pairs.find(std::make_pair(n->Row, n->Col));
                if (v == pairs.end())
                    return false;
                pairs.erase(v);
            }
            n = n->Down;
        }
    }
    return pairs.empty();
}
