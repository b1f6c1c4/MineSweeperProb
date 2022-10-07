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
        gameMgr,
        onProbe,
        onFlag,
    } = props;

    const body = [];
    const best = [], preferred = [];
    if (gameMgr) {
        let set = gameMgr.bestBlocks;
        for (let i = 0; i < set.size(); i++)
            best[set.get(i)] = true;
        set = gameMgr.preferredBlocks;
        for (let i = 0; i < set.size(); i++)
            preferred[set.get(i)] = true;
    }
    const drain = gameMgr && gameMgr.bestProbabilityList;

    for (let i = 0; i < height; i++) {
        const row = [];
        for (let j = 0; j < width; j++) {
            if (!gameMgr) {
                row.push(<Block key={j} />)
                continue;
            }
            const property = gameMgr.blockPropertyOf(j, i);
            const infer = gameMgr.inferredStatusOf(j, i);
            let prob = gameMgr.blockProbabilityOf(j, i);
            if (drain.size())
                prob = 1 - drain.get(j * height + i);
            if (!isStarted)
                prob = null;
            // Note: do NOT use {...property} as it won't work for getters
            row.push(<Block key={j}
                            row={i}
                            col={j}
                            isGameOver={isGameOver}
                            isWon={isWon}
                            degree={property.degree}
                            isOpen={property.isOpen}
                            isFlagged={flagging[j * height + i]}
                            hasMine={property.hasMine}
                            isBest={best[j * height + i]}
                            isPreferred={preferred[j * height + i]}
                            isSafe={infer === module.BlockStatus.BLANK}
                            isDangerous={infer === module.BlockStatus.MINE}
                            isDrain={!!drain.size()}
                            probability={prob}
                            onProbe={onProbe}
                            onFlag={onFlag}
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
