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
    {
        const set = gameMgr.bestBlocks;
        for (let i = 0; i < set.size(); i++)
            best[set.get(i)] = true;
    }
    {
        const set = gameMgr.preferredBlocks;
        for (let i = 0; i < set.size(); i++)
            preferred[set.get(i)] = true;
    }

    for (let i = 0; i < height; i++) {
        const row = [];
        for (let j = 0; j < width; j++) {
            const property = gameMgr.blockPropertyOf(j, i);
            const infer = gameMgr.inferredStatusOf(j, i);
            const prob = gameMgr.blockProbabilityOf(j, i);
            // Note: do NOT use {...property} as it won't work for getters
            row.push(<Block key={j}
                            row={i}
                            col={j}
                            isGameOver={isGameOver}
                            isWon={isWon}
                            degree={property.degree}
                            isOpen={property.isOpen}
                            isFlagged={flagging[i * width + j]}
                            hasMine={property.hasMine}
                            isBest={best[j * height + i]}
                            isPreferred={preferred[j * height + i]}
                            isSafe={infer === module.BlockStatus.BLANK}
                            isDangerous={infer === module.BlockStatus.MINE}
                            probability={isStarted ? prob : null}
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
