import React from 'react';
import Block from './Block';

export default function Board(props) {
    const {
        width,
        height,
        isStarted,
        isGameOver,
        isWon,
        flagging,
        module,
        overlay,
        onProbe,
        onFlag,
        enableAI,
        hoverId,
        onHover,
        onUnHover,
    } = props;
    let {
        gameMgr,
    } = props;

    const body = [];
    const best = [], preferred = [];
    if (enableAI && gameMgr) {
        let set = gameMgr.bestBlocks;
        for (let i = 0; i < set.size(); i++)
            best[set.get(i)] = true;
        set = gameMgr.preferredBlocks;
        for (let i = 0; i < set.size(); i++)
            preferred[set.get(i)] = true;
    }
    const drain = enableAI && gameMgr && gameMgr.bestProbabilityList;

    for (let i = 0; i < height; i++) {
        const row = [];
        for (let j = 0; j < width; j++) {
            if (!gameMgr) {
                row.push(<Block key={j} />)
                continue;
            }
            const property = gameMgr.blockPropertyOf(j, i);
            const infer = enableAI ? gameMgr.inferredStatusOf(j, i) : module.BlockStatus.UNKNOWN;
            const id = j * height + i;
            let prob = null;
            if (enableAI) {
                prob = gameMgr.blockProbabilityOf(j, i);
                if (drain.size())
                    prob = 1 - drain.get(id);
                if (!isStarted)
                    prob = null;
            }
            const o = overlay[id];
            const xo = o !== null && o >= 0 && o <= 8;
            // Note: do NOT use {...property} as it won't work for getters
            row.push(<Block key={j}
                            row={i}
                            col={j}
                            isGameOver={isGameOver}
                            isWon={isWon}
                            degree={xo ? o : property.degree}
                            isOpen={(o !== null && o !== undefined) || property.isOpen}
                            isFlagged={flagging[id]}
                            isEFlagged={o === 'E' || property.degree === -1}
                            hasMine={property.hasMine || o === 'M'}
                            isLastProbe={gameMgr.lastProbe == id}
                            isHover={id == hoverId}
                            isBest={best[id]}
                            isPreferred={preferred[id]}
                            isSafe={infer === module.BlockStatus.BLANK}
                            isDangerous={infer === module.BlockStatus.MINE}
                            isDrain={enableAI && !!drain.size()}
                            probability={prob}
                            onProbe={onProbe}
                            onFlag={onFlag}
                            onHover={() => onHover(id)}
                            onUnHover={onUnHover}
            />);
        }
        body.push(<tr key={i}>{row}</tr>);
    }
    return (<table className="board">
        <tbody>
        {body}
        </tbody>
    </table>);
}
