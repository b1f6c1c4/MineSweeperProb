import React, {useEffect, useState} from 'react';
import Block from './Block';
import HeatBar from './HeatBar';

export default function Board(props) {
    const {
        width,
        height,
        isStarted,
        isGameOver,
        isWon,
        isDrain,
        flagging,
        module,
        overlay,
        onProbe,
        onFlag,
        isExternal,
        enableAI,
        isLocked,
        isAuto,
        isSearching,
        gameMgr,
    } = props;
    const [hoverId, setHoverId] = useState(undefined);

    useEffect(() => {
        if (!isSearching)
            setHoverId(undefined);
    }, [isSearching]);

    const body = [];
    const best = [], preferred = [];
    let hasSafe = false;
    if (enableAI && gameMgr) {
        let set = gameMgr.bestBlocks;
        hasSafe = set.size();
        for (let i = 0; i < set.size(); i++)
            best[set.get(i)] = true;
        set = gameMgr.preferredBlocks;
        if (isAuto)
            for (let i = 0; i < set.size(); i++)
                preferred[set.get(i)] = true;
    }
    let drain = isDrain && gameMgr.bestProbabilityList;
    let hoverInfer = null;
    let hoverProbability = null;

    let minProb = isStarted && !isGameOver && gameMgr ? gameMgr.minProb : 1;
    let maxProb = isStarted && !isGameOver && gameMgr ? gameMgr.maxProb : 0;

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
            let rprob = null;
            if (enableAI) {
                prob = gameMgr.blockProbabilityOf(j, i);
                if (isDrain)
                    prob = drain.get(id);
                if (!isStarted)
                    prob = null;
                if (id === hoverId && !property.isOpen) {
                    hoverInfer = infer;
                    if (infer === module.BlockStatus.UNKNOWN)
                        hoverProbability = prob;
                }
                rprob = minProb < maxProb
                    ? (prob - minProb) / (maxProb - minProb)
                    : prob;
            }
            const o = overlay[id];
            const xo = o !== null && o >= 0 && o <= 8;
            const onLeft = !isLocked && (isSearching ? () => setHoverId(id) : onProbe);
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
                            isLastProbe={gameMgr.lastProbe === id}
                            isHover={id === hoverId && !isExternal}
                            isBest={best[id]}
                            isPreferred={preferred[id]}
                            isSafe={infer === module.BlockStatus.BLANK}
                            isDangerous={infer === module.BlockStatus.MINE}
                            isDrain={isDrain}
                            probability={rprob}
                            onProbe={onLeft}
                            onFlag={!isLocked && onFlag}
                            onHover={() => !isSearching && !isLocked && setHoverId(id)}
                            onUnHover={() => !isSearching && !isLocked && setHoverId(undefined)}
            />);
        }
        body.push(<tr key={i}>{row}</tr>);
    }

    return (<div className="board-container">
        {(isAuto && enableAI && !isExternal) && (
            <HeatBar
                isSafe={hasSafe}
                isDrain={isDrain}
                hasSafe={hasSafe}
                probability={hasSafe ? null : isDrain ? maxProb : minProb}
                minProb={minProb}
                maxProb={maxProb}
            />)}
        {(isExternal || (!isAuto && enableAI)) && (
            <HeatBar
                disabled={isLocked || (isExternal && !enableAI)}
                isSafe={hoverInfer === module.BlockStatus.BLANK}
                isDrain={isDrain}
                isDangerous={hoverInfer === module.BlockStatus.MINE}
                hasSafe={hasSafe}
                probability={hoverProbability}
                minProb={minProb}
                maxProb={maxProb}
            />)}
        <table className="board">
            <tbody>
                {body}
            </tbody>
        </table>
    </div>);
}
