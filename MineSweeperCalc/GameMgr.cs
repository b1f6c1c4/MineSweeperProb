using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Numerics;
using MineSweeperCalc.Solver;

namespace MineSweeperCalc
{
    /// <summary>
    ///     �Զ�ɨ����Ϸ
    /// </summary>
    [Serializable]
    public class GameMgr
    {
        /// <summary>
        ///     ���о���
        /// </summary>
        /// <param name="blocks">��</param>
        /// <param name="mgr">��Ϸ</param>
        /// <returns>���Ÿ�</returns>
        public delegate IEnumerable<Block> DecideDelegate(
            List<Block> blocks, GameMgr mgr, bool multiThread,
            IDictionary<Block, IDictionary<int, BigInteger>> dist = null);

        /// <summary>
        ///     ��
        /// </summary>
        private readonly Block[,] m_Blocks;

        /// <summary>
        ///     ���
        /// </summary>
        public int TotalWidth { get; }

        /// <summary>
        ///     �߶�
        /// </summary>
        public int TotalHeight { get; }

        /// <summary>
        ///     ������
        /// </summary>
        public int TotalMines { get; }

        /// <summary>
        ///     �Ѳ���
        /// </summary>
        private bool m_Settled;

        /// <summary>
        ///     �Ƿ��ѿ�ʼ
        /// </summary>
        public bool Started { get; private set; }

        /// <summary>
        ///     �Ƿ�ʤ��
        /// </summary>
        public bool Succeed { get; private set; }

        /// <summary>
        ///     ����������
        /// </summary>
        // ReSharper disable once MemberCanBePrivate.Global
        public int ToOpen { get; private set; }

        /// <summary>
        ///     �����
        /// </summary>
        public Solver<Block> Solver { get; }

        /// <summary>
        ///     ����
        /// </summary>
        public virtual IDictionary<Block, double> Probability => Solver.Probability;

        /// <summary>
        ///     ��״̬��
        /// </summary>
        public virtual BigInteger TotalStates => Solver.TotalStates;

        /// <summary>
        ///     ������
        /// </summary>
        // ReSharper disable once MemberCanBePrivate.Global
        public DecideDelegate DecisionMaker { get; set; }

        /// <summary>
        ///     �����������
        /// </summary>
        private readonly Random m_Random;

        public GameMgr(int width, int height, int totalMines, int seed, DecideDelegate decisionMaker = null)
        {
            m_Blocks = new Block[width, height];
            TotalWidth = width;
            TotalHeight = height;
            TotalMines = totalMines;
            m_Random = new Random(seed);
            DecisionMaker = decisionMaker;

            var set = new Collection<Block>();

            for (var i = 0; i < width; i++)
                for (var j = 0; j < height; j++)
                {
                    var block = new Block(i, j, this);
                    set.Add(block);
                    m_Blocks[i, j] = block;
                }

            for (var i = 0; i < width; i++)
                for (var j = 0; j < height; j++)
                {
                    var lst = new List<Block>(8);
                    for (var di = -1; di <= 1; di++)
                        if (i + di >= 0 &&
                            i + di < width)
                            for (var dj = -1; dj <= 1; dj++)
                                if (j + dj >= 0 &&
                                    j + dj < height)
                                    if (di != 0 ||
                                        dj != 0)
                                        lst.Add(m_Blocks[i + di, j + dj]);
                    m_Blocks[i, j].Surrounding = new BlockSet<Block>(lst);
                }

            Solver = new Solver<Block>(set);
            Solver.AddRestrain(new BlockSet<Block>(set), TotalMines);

            Started = true;
            Succeed = false;
            ToOpen = TotalWidth * TotalHeight - TotalMines;
        }

        /// <summary>
        ///     �ܿ�ĳ������
        /// </summary>
        /// <param name="initX">�ܿ��ĸ�ĺ�����</param>
        /// <param name="initY">�ܿ��ĸ��������</param>
        private void SettleMines(int initX, int initY)
        {
            var totalMines = TotalMines;
            while (totalMines > 0)
            {
                var x = m_Random.Next(TotalWidth);
                var y = m_Random.Next(TotalHeight);
                if (x == initX &&
                    y == initY)
                    continue;
                if (m_Blocks[x, y].IsMineInternal())
                    continue;
                m_Blocks[x, y].IsMine = true;
                totalMines--;
            }
            m_Settled = true;
        }

        /// <summary>
        ///     ��ȡĳһλ�õĸ�
        /// </summary>
        /// <param name="x">������</param>
        /// <param name="y">������</param>
        /// <returns>��</returns>
        public Block this[int x, int y] => m_Blocks[x, y];

        /// <summary>
        ///     ����ĳһλ�ã��緭��0���Զ������ܱ�λ��
        /// </summary>
        /// <param name="x">������</param>
        /// <param name="y">������</param>
        public void OpenBlock(int x, int y)
        {
            if (!Started)
                throw new InvalidOperationException("��Ϸ�ѽ���");
            if (!m_Settled)
                SettleMines(x, y);

            var block = m_Blocks[x, y];
            if (block.IsOpen)
                throw new InvalidOperationException("�˸��ѷ���");
            block.IsOpen = true;

            if (block.IsMine)
            {
                Started = false;
                return;
            }

            Solver.AddRestrain(new BlockSet<Block>(block), 0);
            if (block.Degree == 0)
                foreach (var b in block.Surrounding.Blocks.Where(b => !b.IsOpen))
                    OpenBlock(b.X, b.Y);
            else
                Solver.AddRestrain(block.Surrounding, block.Degree);

            if (--ToOpen == 0)
            {
                Started = false;
                Succeed = true;
            }
        }

        /// <summary>
        ///     ��ȡȷ�����׵Ŀɷ����ĸ�
        /// </summary>
        /// <returns>ȷ�����׵Ŀɷ����ĸ�</returns>
        public IEnumerable<Block> CanOpenForSureBlocks()
        {
            for (var i = 0; i < TotalWidth; i++)
                for (var j = 0; j < TotalHeight; j++)
                    if (!m_Blocks[i, j].IsOpen &&
                        Solver[m_Blocks[i, j]] == BlockStatus.Blank)
                        yield return m_Blocks[i, j];
        }

        /// <summary>
        ///     ��ȡ��һ�����׵Ŀɷ����ĸ�
        /// </summary>
        /// <returns>��һ�����׵Ŀɷ����ĸ�</returns>
        public IEnumerable<Block> CanOpenNotSureBlocks()
        {
            for (var i = 0; i < TotalWidth; i++)
                for (var j = 0; j < TotalHeight; j++)
                    if (!m_Blocks[i, j].IsOpen &&
                        Solver[m_Blocks[i, j]] == BlockStatus.Unknown)
                        yield return m_Blocks[i, j];
        }

        /// <summary>
        ///     ���Զ�����һ��
        /// </summary>
        /// <param name="withProb">������</param>
        /// <returns>���Լ������Զ�����</returns>
        public bool SemiAutomaticStep(bool withProb)
        {
            if (!Started)
                return false;
            Solver.Solve(withProb);
            var flag = false;
            foreach (var block in CanOpenForSureBlocks())
            {
                OpenBlock(block.X, block.Y);
                flag = true;
                if (Started)
                    continue;
                if (!Succeed)
                    throw new ApplicationException("�жϴ���");
                break;
            }
            return flag && Started;
        }

        /// <summary>
        ///     ���Զ�����
        /// </summary>
        /// <returns>��Ҫ���о���</returns>
        public virtual bool SemiAutomatic()
        {
            if (!Started)
                return false;
            while (true)
            {
                while (SemiAutomaticStep(false)) { }
                if (!SemiAutomaticStep(true))
                    break;
            }
            return Started;
        }

        /// <summary>
        ///     ���ض������Զ�����һ��
        /// </summary>
        /// <param name="multiThread"></param>
        public virtual void AutomaticStep(bool multiThread)
        {
            if (!Started)
                return;

            if (DecisionMaker == null)
            {
                Started = false;
                return;
            }

            var ary = DecisionMaker(CanOpenNotSureBlocks().ToList(), this, multiThread).ToArray();
            var blk = ary[m_Random.Next(ary.Length)];
            OpenBlock(blk.X, blk.Y);
        }

        /// <summary>
        ///     ���ض������Զ�����
        /// </summary>
        /// <param name="multiThread"></param>
        public virtual void Automatic(bool multiThread)
        {
            while (Started)
                if (SemiAutomatic())
                    AutomaticStep(multiThread);
        }
    }
}
