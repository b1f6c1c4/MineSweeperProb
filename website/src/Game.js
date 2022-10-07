import React, {useEffect, useReducer, useRef, useState} from 'react';
import Board from './Board';
import {
    Button,
    ButtonGroup,
    Card,
    ControlGroup,
    Elevation,
    FormGroup,
    ProgressBar,
    Slider,
    Switch
} from '@blueprintjs/core';

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
    const [isAutoFlag, setIsAutoFlag] = useState(true);
    const isAutoFlagRef = useRef(isAutoFlag);
    isAutoFlagRef.current = isAutoFlag;

    const [, forceUpdate] = useReducer(x => x + 1, 0);
    const [isSettled, setIsSettled] = useState(false);
    const [isGameOver, setIsGameOver] = useState(false);
    const [isWon, setIsWon] = useState(false);
    const [rate, setRate] = useState([1, 1]);
    const [toOpen, setToOpen] = useState(width * height - totalMines);
    const [hasBest, setHasBest] = useState(false);

    const [flagging, setFlagging] = useState([]);
    const flaggingRef = useRef(flagging);
    flaggingRef.current = flagging;
    const [totalFlagged, setTotalFlagged] = useState(0);
    const totalFlaggedRef = useRef(totalFlagged);
    totalFlaggedRef.current = totalFlagged;
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
            setHasBest(false);
            if (isAutoRestartRef.current)
                setTimeout(() => {
                    // just in case the user cancels during the period
                    if (isAutoRestartRef.current)
                        onRestart();
                }, 1500);
        } else {
            gameMgrRef.current.solve(module.SolvingState.AUTO, false);
        }
        if (isAutoFlagRef.current) {
            const next = flaggingRef.current;
            let tf = totalFlaggedRef.current;
            for (let i = 0; i < height; i++)
                for (let j = 0; j < width; j++) {
                    if (gameMgrRef.current.inferredStatusOf(j, i) === module.BlockStatus.MINE)
                        if (!next[j * height + i]) {
                            next[j * height + i] = true;
                            tf++;
                        }
                    if (gameMgrRef.current.blockPropertyOf(j, i).isOpen)
                        if (next[j * height + i]) {
                            delete next[j * height + i];
                            tf--;
                        }
                }
            setFlagging(next);
            setTotalFlagged(tf);
        }
        setHasBest(!!gameMgrRef.current.bestBlocks.size());
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
        setTotalFlagged(0);
        setRate([1, 1]);
        setHasBest(false);
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
        if (f[col * height + row]) {
            delete f[col * height + row];
            setTotalFlagged(totalFlagged - 1);
        } else {
            f[col * height + row] ^= true;
            setTotalFlagged(totalFlagged + 1);
        }
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

    function onSwitchFlag(e) {
        setIsAutoFlag(e.currentTarget.checked);
    }

    return (
        <div>
            <Card elevation={Elevation.TWO}>
                <div>
                <p>{`${totalFlagged} / ${totalMines} mines flagged`}</p>
                <ProgressBar value={totalFlagged / totalMines} stripes={!isGameOver}
                             intent={(isGameOver && !isWon) ? 'danger' : 'warning'} />
                </div>
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
                />
                <div>
                <p>{`${Math.round(Math.pow(2, rate[0]))} possible solutions`}</p>
                <ProgressBar value={1 - rate[0] / rate[1]} stripes={!isGameOver}
                             intent={(isGameOver && !isWon) ? 'danger' : 'success'} />
                <p>{`${toOpen} / ${width * height - totalMines} blocks left`}</p>
                <ProgressBar value={1 - toOpen / (width * height - totalMines)}
                             stripes={!isGameOver}
                             intent={(isGameOver && !isWon) ? 'danger' : 'primary'} />
                </div>
            </Card>
            <FormGroup label="Game Control">
                <ControlGroup>
                    <ButtonGroup>
                        <Button disabled={!gameMgr || mode != null}
                                icon="reset" intent="danger"
                                text="Restart" onClick={onRestart} />
                    </ButtonGroup>
                    <Switch checked={isAutoRestart} onChange={onSwitch}
                            labelElement={'Auto-restart'}
                            innerLabelChecked="on" innerLabel="off" />
                </ControlGroup>
            </FormGroup>
            <FormGroup label="AI Control">
                <ButtonGroup>
                    <Button disabled={!gameMgr || isGameOver || mode !== null}
                            icon="hand-up" intent="primary"
                            text="Single step" onClick={onStep} />
                </ButtonGroup>
                <ControlGroup>
                    <ButtonGroup>
                        <Button disabled={!gameMgr || isGameOver || (mode !== null && mode !== 'semi') || !hasBest}
                                active={mode === 'semi'}
                                icon="play" intent="success"
                                text="Semi-auto" onClick={onSemi} />
                        <Button disabled={!gameMgr || isGameOver || mode !== null || !hasBest}
                                icon="fast-forward" intent="success"
                                onClick={onSemiAll} />
                    </ButtonGroup>
                    <ButtonGroup>
                        <Button disabled={!gameMgr || isGameOver || (mode !== null && mode !== 'auto')}
                                active={mode === 'auto'}
                                icon="fast-forward" intent="warning"
                                text="Full-auto" onClick={onAuto} />
                        <Button disabled={!gameMgr || isGameOver || mode !== null}
                                icon="lightning" intent="warning"
                                onClick={onAutoAll} />
                    </ButtonGroup>
                </ControlGroup>
                <ControlGroup>
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
                </ControlGroup>
                <Switch checked={isAutoFlag} onChange={onSwitchFlag}
                        labelElement={'Auto-flag'}
                        innerLabelChecked="on" innerLabel="off" />
            </FormGroup>
        </div>
    );
}
