#pragma once
#include "stdafx.h"
#include <vector>
#include <algorithm>

template <class T>
struct Node
{
    int Row, Col;
    T Value;
    Node *Right, *Down;

    Node();
    Node(int row, int col);
    Node(int row, int col, T &&val);
};

template <class T>
class DLL_API OrthogonalList
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

    Node<T> &Add(int row, int col, T &&val);
    Node<T> &Add(Node<T> &nr, Node<T> &nc, T &&val);

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
Node<T>::Node() : Row(-1), Col(-1) {}

template <class T>
Node<T>::Node(int row, int col) : Row(row), Col(col) {}

template <class T>
Node<T>::Node(int row, int col, T &&val) : Row(row), Col(col), Value(val) {}

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
        const auto n = other.GetRowHead(i).Right;
        auto nr = m_Rows[i];
        while (n != nullptr)
            nr = Add(nr, m_Cols[n->Col], std::move(n->Value));
    }
}

template <class T>
OrthogonalList<T>::~OrthogonalList()
{
    std::for_each(m_Rows.begin(), m_Rows.end(), [](Node<T> &node)
                  {
                      auto n = node.Right;
                      while (n != nullptr)
                      {
                          auto tmp = n->Right;
                          delete n;
                          n = tmp;
                      }
                  });
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
    while (true)
    {
        if (nr.Right == nullptr || nr.Right->Col >= col)
            break;
        nr = *nr.Right;
    }
    return nr;
}

template <class T>
Node<T> &OrthogonalList<T>::SeekDown(int row, int col)
{
    return SeekDown(row, m_Cols[col]);
}

template <class T>
Node<T> &OrthogonalList<T>::SeekDown(int row, Node<T> &nc)
{
    while (true)
    {
        if (nc.Down == nullptr || nc.Down->Row >= row)
            break;
        nc = *nc.Down;
    }
    return nc;
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
Node<T> &OrthogonalList<T>::Add(int row, int col, T &&val)
{
    return Add(GetRowHead(row), GetColHead(col), std::move(val));
}

template <class T>
Node<T> &OrthogonalList<T>::Add(Node<T> &nr, Node<T> &nc, T &&val)
{
    nr = SeekRight(nr, nc.Col);
    if (nr.Down != nullptr && nr.Down->Row == nr.Row)
    {
        nr.Down->Value = val;
        return *nr.Down;
    }
    nc = SeekDown(nr.Row, nc);

    Node<T> *node = new Node<T>(nr.Row, nc.Col, std::move(val));
    node->Right = nr.Right;
    node->Down = nc.Down;
    nr.Right = node;
    nc.Down = node;

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
    auto left = SeekRight(nr, nc.Col);
    auto up = SeekDown(nr.Row, nc);

    if (left.Right == nullptr || up.Down == nullptr)
        return;

    auto node = left.Right;
    if (node->Col != nc.Col)
        return;

    left.Right = node->Right;
    up.Down = node->Down;

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
    auto nr = m_Rows[row];
    while (nr.Right != nullptr)
        Remove(nr, m_Cols[nr.Right->Col]);
    std::for_each(m_Rows.begin() + row + 1, m_Rows.end(), [](Node<T> &node)
                  {
                      --node.Row;
                      auto n = node.Right;
                      while (n != nullptr)
                      {
                          --n->Row;
                          n = n->Right;
                      }
                  });
    m_Rows.erase(m_Rows.begin() + row);
}

template <class T>
void OrthogonalList<T>::RemoveCol(int col)
{
    auto nc = m_Cols[col];
    while (nc.Down != nullptr)
        Remove(m_Rows[nc.Down->Row], nc);
    std::for_each(m_Cols.begin() + col + 1, m_Cols.end(), [](Node<T> &node)
                  {
                      --node.Col;
                      auto n = node.Down;
                      while (n != nullptr)
                      {
                          --n->Col;
                          n = n->Down;
                      }
                  });
    m_Cols.erase(m_Cols.begin() + col);
}

template <class T>
void OrthogonalList<T>::InsertRow(int row)
{
    std::for_each(m_Rows.begin() + row, m_Rows.end(), [](Node<T> &node)
                  {
                      ++node.Row;
                      auto n = node.Right;
                      while (n != nullptr)
                      {
                          ++n->Row;
                          n = n->Right;
                      }
                  });
    m_Rows.insert(m_Rows.begin() + row, Node<T>(row, -1));
}

template <class T>
void OrthogonalList<T>::InsertCol(int col)
{
    std::for_each(m_Cols.begin() + col, m_Cols.end(), [](Node<T> &node)
                  {
                      ++node.Col;
                      auto n = node.Down;
                      while (n != nullptr)
                      {
                          ++n->Col;
                          n = n->Down;
                      }
                  });
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
        m_Rows.push_back(std::move(Node<T>(i, -1)));
}

template <class T>
void OrthogonalList<T>::ExtendWidth(int cols)
{
    if (m_Cols.size() >= cols)
        return;

    auto size = m_Cols.size();

    m_Cols.reserve(cols);

    for (auto i = size; i < cols; ++i)
        m_Cols.push_back(std::move(Node<T>(i, -1)));
}
