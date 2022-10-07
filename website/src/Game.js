import React, {useEffect, useReducer, useRef, useState} from 'react';
import Board from './Board';
import {Button, ButtonGroup, ProgressBar, Slider, Switch} from '@blueprintjs/core';

export default function Game(props) {
    const {
        module,
        width,
        height,
        totalMines,
        strategy,
    } = props;
    const [gameMgr, setGameMgr] = useState(undefined);
    const gameMgrRef = useRef(gameMgr);
    gameMgrRef.current = gameMgr;

    // could be inside setTimeout
    function startGame() {
        const cfg = module.parse(strategy);
        module.cache(cfg.width, cfg.height, cfg.totalMines);
        const mgr = new module.GameMgr(cfg.width, cfg.height, cfg.totalMines, cfg.isSNR, cfg, false);
        setGameMgr(mgr);
        setRate([mgr.bits, mgr.allBits]);
        setToOpen(width * height - totalMines);
    }

    useEffect(startGame, [strategy]);

    const [isAutoRestart, setIsAutoRestart] = useState(false);
    const isAutoRestartRef = useRef(isAutoRestart);
    isAutoRestartRef.current = isAutoRestart;

    const [, forceUpdate] = useReducer(x => x + 1, 0);
    const [isSettled, setIsSettled] = useState(false);
    const [isGameOver, setIsGameOver] = useState(false);
    const [isWon, setIsWon] = useState(false);
    const [rate, setRate] = useState([1, 1]);
    const [toOpen, setToOpen] = useState(width * height - totalMines);

    const [flagging, setFlagging] = useState([]);
    const [speed, setSpeed] = useState(-1.5);
    const speedRef = useRef(speed);
    speedRef.current = speed;
    const [mode, setMode] = useState(null);
    const modeRef = useRef(mode);
    modeRef.current = mode;
    const [canceller, setCanceller] = useState(undefined);
    const cancellerRef = useRef(canceller);
    cancellerRef.current = canceller;

    // could be inside setTimeout
    function onUpdate() {
        if (!gameMgrRef.current.started) {
            setIsGameOver(true);
            setIsWon(gameMgrRef.current.succeed);
            if (isAutoRestartRef.current)
                setTimeout(() => {
                    // just in case the user cancels during the period
                    if (isAutoRestartRef.current)
                        onRestart();
                }, 1500);
        } else {
            gameMgrRef.current.solve(module.SolvingState.AUTO, false);
        }
        setIsSettled(gameMgrRef.current.settled);
        setRate([gameMgrRef.current.bits, gameMgrRef.current.allBits]);
        setToOpen(gameMgrRef.current.toOpen);
        forceUpdate();
    }

    // could be inside setTimeout
    function onRestart() {
        const m = modeRef.current;
        setIsSettled(false);
        setIsGameOver(false);
        setIsWon(false);
        setFlagging([]);
        setRate([1, 1]);
        setToOpen(width * height - totalMines)
        if (cancellerRef.current) {
            clearTimeout(cancellerRef.current);
            setCanceller(undefined);
        }
        setMode(null);
        startGame(module);
        if (m === 'auto')
            setTimeout(onAuto, 500);
        else if (m === 'auto-all')
            setTimeout(onAutoAll, 500);
    }

    function onProbe(row, col) {
        gameMgr.openBlock(col, row);
        onUpdate();
    }

    function onFlag(row, col) {
        const f = [...flagging];
        f[col * height + row] ^= true;
        setFlagging(f);
    }

    // could be inside setTimeout
    function onStep() {
        if (!gameMgrRef.current.semiAutomaticStep(module.SolvingState.SEMI, true))
            gameMgrRef.current.automaticStep(module.SolvingState.AUTO);
        onUpdate();
    }

    function onSemi() {
        if (mode === null) {
            setMode('semi');
            // could be inside setTimeout
            const foo = () => {
                setCanceller(undefined);
                const next = gameMgrRef.current.semiAutomaticStep(module.SolvingState.SEMI, true);
                onUpdate();
                if (next)
                    setCanceller(setTimeout(foo, Math.pow(10, 3 + speedRef.current)));
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

    function onSemiAll() {
        gameMgr.semiAutomatic(module.SolvingState.SEMI);
        onUpdate();
    }

    // could be inside setTimeout
    function onAuto() {
        if (modeRef.current === null) {
            setMode('auto');
            // could be inside setTimeout
            const foo = () => {
                setCanceller(undefined);
                onStep();
                if (gameMgrRef.current.started)
                    setCanceller(setTimeout(foo, Math.pow(10, 3 + speedRef.current)));
                else if (!isAutoRestartRef.current)
                    setMode(null);
            };
            foo();
        } else {
            clearTimeout(cancellerRef.current);
            setCanceller(undefined);
            setMode(null);
        }
    }

    // could be inside setTimeout
    function onAutoAll() {
        gameMgrRef.current.automatic();
        if (isAutoRestartRef.current)
            setMode('auto-all');
        onUpdate();
    }

    function renderLabel(v) {
        const y = Math.pow(10, -v);
        const s = Math.pow(10, Math.ceil(v) + 1);
        return `${Math.round(y * s) / s}x`;
    }

    function onSwitch(e) {
        setIsAutoRestart(e.currentTarget.checked);
        if (isGameOver && !e.currentTarget.checked)
            setMode(null);
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
                            icon="hand-up" intent="primary"
                            text="Single step" onClick={onStep} />
                    <Button disabled={!gameMgr || isGameOver || (mode !== null && mode !== 'semi')}
                            active={mode === 'semi'}
                            icon="play" intent="success"
                            text="Semi-auto" onClick={onSemi} />
                    <Button disabled={!gameMgr || isGameOver || mode !== null}
                            icon="fast-forward" intent="success"
                            onClick={onSemiAll} />
                    <Button disabled={!gameMgr || isGameOver || (mode !== null && mode !== 'auto')}
                            active={mode === 'auto'}
                            icon="fast-forward" intent="warning"
                            text="Full-auto" onClick={onAuto} />
                    <Button disabled={!gameMgr || isGameOver || mode !== null}
                            icon="lightning" intent="warning"
                            onClick={onAutoAll} />
                    <Switch checked={isAutoRestart} onChange={onSwitch}
                            labelElement={'Auto-restart'}
                            innerLabelChecked="on" innerLabel="off" />
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
