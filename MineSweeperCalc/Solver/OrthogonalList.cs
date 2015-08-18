using System;
using System.Collections.Generic;
using System.Text;

namespace MineSweeperCalc.Solver
{
    /// <summary>
    ///     十字链表
    /// </summary>
    internal class OrthogonalList<T> //: IDisposable
    {
        /// <summary>
        ///     节点
        /// </summary>
        public class Node
        {
            public Node(int row, int col)
            {
                Row = row;
                Col = col;
            }

            public Node(int row, int col, T val) : this(row, col) { Val = val; }

            public int Row { get; internal set; }
            public int Col { get; internal set; }
            public T Val { get; internal set; }

            public Node Left { get; internal set; }
            public Node Right { get; internal set; }
            public Node Up { get; internal set; }
            public Node Down { get; internal set; }

            public override string ToString() => $"{{{Row},{Col}}}->{Val}";
        }

        private readonly Node m_Root;
        private readonly List<Node> m_Rows;
        private readonly List<Node> m_Cols;

        public int Height => m_Rows.Count;
        public int Width => m_Cols.Count;

        public OrthogonalList()
        {
            m_Root = new Node(-1, -1);
            m_Rows = new List<Node>();
            m_Cols = new List<Node>();
        }

        public OrthogonalList(int width, int height) : this()
        {
            ExtendHeight(height);
            ExtendWidth(width);
        }

        /// <summary>
        ///     扩充行数
        /// </summary>
        /// <param name="rows">总行数</param>
        private void ExtendHeight(int rows)
        {
            if (Height > rows)
                return;

            m_Rows.Capacity = rows;
            if (Height == 0)
                m_Rows.Add(m_Root.Down = new Node(0, -1) { Up = m_Root });

            for (var i = Height; i < rows; i++)
                m_Rows.Add(m_Rows[i - 1].Down = new Node(i, -1) { Up = m_Rows[i - 1] });
        }

        /// <summary>
        ///     扩充列数
        /// </summary>
        /// <param name="cols">总列数</param>
        private void ExtendWidth(int cols)
        {
            if (Width > cols)
                return;

            m_Cols.Capacity = cols;
            if (Width == 0)
                m_Cols.Add(m_Root.Right = new Node(-1, 0) { Left = m_Root });

            for (var i = Width; i < cols; i++)
                m_Cols.Add(m_Cols[i - 1].Right = new Node(-1, i) { Left = m_Cols[i - 1] });
        }

        /// <summary>
        ///     横向定位
        /// </summary>
        /// <param name="row">行</param>
        /// <param name="col">列</param>
        /// <param name="nr">左侧节点</param>
        /// <returns>节点</returns>
        public Node SeekRight(int row, int col, out Node nr)
        {
            nr = m_Rows[row];
            while (true)
            {
                if (nr.Right == null ||
                    nr.Right.Col >= col)
                    break;
                nr = nr.Right;
            }
            if (nr.Right == null ||
                nr.Right.Col > col)
                return null;
            return nr.Right;
        }

        /// <summary>
        ///     纵向定位
        /// </summary>
        /// <param name="row">行</param>
        /// <param name="col">列</param>
        /// <param name="nc">上侧节点</param>
        /// <returns>节点</returns>
        public Node SeekDown(int row, int col, out Node nc)
        {
            nc = m_Cols[col];
            while (true)
            {
                if (nc.Down == null ||
                    nc.Down.Row >= row)
                    break;
                nc = nc.Down;
            }
            if (nc.Down == null ||
                nc.Down.Row > row)
                return null;
            return nc.Down;
        }

        /// <summary>
        ///     定位
        /// </summary>
        /// <param name="row">行</param>
        /// <param name="col">列</param>
        /// <param name="nr">左侧节点</param>
        /// <param name="nc">上侧节点</param>
        /// <returns>节点</returns>
        public Node Seek(int row, int col, out Node nr, out Node nc)
        {
            var node = SeekDown(row, col, out nc);
            if (node != null)
            {
                nr = node.Left;
                return node;
            }
            SeekRight(row, col, out nr);
            return null;
        }

        /// <summary>
        ///     添加或更新
        /// </summary>
        /// <param name="row">行</param>
        /// <param name="col">列</param>
        /// <param name="val">值</param>
        /// <param name="update">更新</param>
        /// <returns>节点</returns>
        public Node AddOrUpdate(int row, int col, T val, Func<T, T> update)
        {
            ExtendHeight(row);
            ExtendWidth(col);

            Node nr, nc;
            var node = Seek(row, col, out nr, out nc);
            if (node != null)
            {
                node.Val = update(node.Val);
                return node;
            }

            var newNode = new Node(row, col, val) { Left = nr, Right = nr.Right, Up = nc, Down = nc.Down };
            if (nr.Right != null)
                nr.Right.Left = newNode;
            if (nc.Down != null)
                nc.Down.Up = newNode;
            nc.Down = nr.Right = newNode;
            return newNode;
        }

        /// <summary>
        ///     获取或添加
        /// </summary>
        /// <param name="row">行</param>
        /// <param name="col">列</param>
        /// <param name="add">添加</param>
        /// <returns>节点</returns>
        public Node GetOrAdd(int row, int col, Func<T> add = null)
        {
            ExtendHeight(row);
            ExtendWidth(col);

            if (row == -1 ||
                col == -1)
            {
                if (row == -1 &&
                    col == -1)
                    return m_Root;
                return row == -1 ? m_Cols[col] : m_Rows[row];
            }

            Node nr, nc;
            var node = Seek(row, col, out nr, out nc);
            if (node != null)
                return node;

            if (add == null)
                return null;

            var newNode = new Node(row, col, add()) { Left = nr, Right = nr.Right, Up = nc, Down = nc.Down };
            if (nr.Right != null)
                nr.Right.Left = newNode;
            if (nc.Down != null)
                nc.Down.Up = newNode;
            nc.Down = nr.Right = newNode;
            return newNode;
        }

        /// <summary>
        ///     遍历行
        /// </summary>
        /// <param name="row">行</param>
        /// <returns>该行节点</returns>
        public IEnumerable<Node> GetRow(int row)
        {
            var node = m_Rows[row];
            while (true)
            {
                node = node.Right;
                if (node == null)
                    break;
                yield return node;
            }
        }

        /// <summary>
        ///     遍历列
        /// </summary>
        /// <param name="col">列</param>
        /// <returns>该列节点</returns>
        public IEnumerable<Node> GetCol(int col)
        {
            var node = m_Cols[col];
            while (true)
            {
                node = node.Down;
                if (node == null)
                    break;
                yield return node;
            }
        }

        /// <summary>
        ///     移除节点
        /// </summary>
        /// <param name="node">节点</param>
        public void Remove(Node node)
        {
            node.Left.Right = node.Right;
            node.Up.Down = node.Down;
            if (node.Right != null)
                node.Right.Left = node.Left;
            if (node.Down != null)
                node.Down.Up = node.Up;

            node.Up = null;
            node.Left = null;
        }

        public Node this[int row, int col] => GetOrAdd(row, col);

        public override string ToString()
        {
            var sb = new StringBuilder();
            sb.Append("{");
            for (int i = 0; i < Height; i++)
                foreach (var node in GetRow(i))
                {
                    sb.Append(node);
                    sb.Append(",");
                }
            if (sb.Length > 1)
                sb.Remove(sb.Length - 1, 1);
            sb.Append("}");
            return sb.ToString();
        }
    }
}
