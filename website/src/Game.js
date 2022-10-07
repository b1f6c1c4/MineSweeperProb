import React from 'react';
import Board from './Board';
import {useEffect, useReducer, useState} from 'react';
import {Button, ButtonGroup, ProgressBar, Slider} from '@blueprintjs/core';

export default function Game(props) {
    const {
        module,
        width,
        height,
        totalMines,
        strategy,
    } = props;
    const [gameMgr, setGameMgr] = useState(undefined);

    function startGame() {
        const cfg = module.parse(strategy);
        module.cache(cfg.width, cfg.height, cfg.totalMines);
        const mgr = new module.GameMgr(cfg.width, cfg.height, cfg.totalMines, cfg.isSNR, cfg, false);
        setGameMgr(mgr);
        setRate([mgr.bits, mgr.allBits]);
    }

    useEffect(startGame, [strategy]);

    const [, forceUpdate] = useReducer(x => x + 1, 0);
    const [isSettled, setIsSettled] = useState(false);
    const [isGameOver, setIsGameOver] = useState(false);
    const [isWon, setIsWon] = useState(false);
    const [rate, setRate] = useState([1, 1]);
    const [toOpen, setToOpen] = useState(width * height - totalMines);
    function onUpdate() {
        if (!gameMgr.started) {
            setIsGameOver(true);
            setIsWon(gameMgr.succeed);
        } else {
            gameMgr.solve(module.SolvingState.AUTO, false);
        }
        setIsSettled(gameMgr.settled);
        console.log(gameMgr.bits);
        setRate([gameMgr.bits, gameMgr.allBits]);
        setToOpen(gameMgr.toOpen);
        forceUpdate();
    }

    function onRestart() {
        setIsSettled(false);
        setIsGameOver(false);
        setIsWon(false);
        setFlagging([]);
        setRate([1, 1]);
        setMode(null);
        setToOpen(width * height - totalMines)
        if (canceller) {
            clearTimeout(canceller);
            setCanceller(undefined);
        }
        startGame(module);
    }

    function onProbe(row, col) {
        gameMgr.openBlock(col, row);
        onUpdate();
    }
    const [flagging, setFlagging] = useState([]);
    function onFlag(row, col) {
        const f = [...flagging];
        f[col * height + row] ^= true;
        setFlagging(f);
    }

    function onStep() {
        if (!gameMgr.semiAutomaticStep(module.SolvingState.SEMI, true))
            gameMgr.automaticStep(module.SolvingState.AUTO);
        onUpdate();
    }

    const [speed, setSpeed] = useState(-1.5);
    const [mode, setMode] = useState(null);
    const [canceller, setCanceller] = useState(undefined);
    function onSemi() {
        if (mode === null) {
            setMode('semi');
            const foo = () => {
                setCanceller(undefined);
                const next = gameMgr.semiAutomaticStep(module.SolvingState.SEMI, true);
                onUpdate();
                if (next)
                    setCanceller(setTimeout(foo, Math.pow(10, 3 + speed)));
                else
                    setMode(null);
            };
            foo();
        } else {
            clearTimeout(canceller);
            setCanceller(undefined);
            setMode(null);
        }
    }
    function onAuto() {
        if (mode === null) {
            setMode('auto');
            const foo = () => {
                setCanceller(undefined);
                onStep();
                if (gameMgr.started)
                    setCanceller(setTimeout(foo, Math.pow(10, 3 + speed)));
                else
                    setMode(null);
            };
            foo();
        } else {
            clearTimeout(canceller);
            setCanceller(undefined);
            setMode(null);
        }
    }

    function renderLabel(v) {
        const y = Math.pow(10, -v);
        const s = Math.pow(10, Math.ceil(v) + 1);
        return `${Math.round(y * s) / s}x`;
    }

    return (
        <div>
            {gameMgr && (
                <Board
                    width={width}
                    height={height}
                    isStarted={isSettled}
                    isGameOver={isGameOver}
                    isWon={isWon}
                    gameMgr={gameMgr}
                    module={module}
                    flagging={flagging}
                    onProbe={onProbe}
                    onFlag={onFlag}
                />)}
            <div>
                <p>{`${Math.round(Math.pow(2, rate[0]))} possible solutions`}</p>
                <ProgressBar value={1 - rate[0] / rate[1]} stripes={!isGameOver}
                             intent={(isGameOver && !isWon) ? 'danger' : 'success'} />
                <p>{`${toOpen} / ${width * height - totalMines} blocks left`}</p>
                <ProgressBar value={1 - toOpen / (width * height - totalMines)}
                             stripes={!isGameOver}
                             intent={(isGameOver && !isWon) ? 'danger' : 'success'} />
                <ButtonGroup>
                    <Button disabled={!gameMgr || mode != null}
                            icon="reset" intent="danger"
                            text="Restart" onClick={onRestart} />
                    <Button disabled={!gameMgr || isGameOver || mode !== null}
                            icon="step-forward" intent="primary"
                            text="Single step" onClick={onStep} />
                    <Button disabled={!gameMgr || isGameOver || mode === 'auto'}
                            active={mode === 'semi'}
                            icon="play" intent="success"
                            text="Semi-auto" onClick={onSemi} />
                    <Button disabled={!gameMgr || isGameOver || mode === 'semi'}
                            active={mode === 'auto'}
                            icon="fast-forward" intent="warning"
                            text="Full-auto" onClick={onAuto} />
                </ButtonGroup>
                <Slider
                    min={-2}
                    max={2}
                    stepSize={0.1}
                    labelStepSize={1}
                    onChange={setSpeed}
                    labelRenderer={renderLabel}
                    showTrackFill={false}
                    value={speed}
                />
            </div>
        </div>
    );
}
