import React, {useEffect, useReducer, useRef, useState} from 'react';
import Board from './Board';
import {
    Alignment,
    Button,
    ButtonGroup,
    Card, Collapse,
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
        config,
        onStop,
    } = props;
    const [gameMgr, setGameMgr] = useState(undefined);
    const gameMgrRef = useRef(gameMgr);
    gameMgrRef.current = gameMgr;

    const [isAutoRestart, setIsAutoRestart] = useState(false);
    const isAutoRestartRef = useRef(isAutoRestart);
    isAutoRestartRef.current = isAutoRestart;
    const [isAutoFlag, setIsAutoFlag] = useState(true);
    const isAutoFlagRef = useRef(isAutoFlag);
    isAutoFlagRef.current = isAutoFlag;
    const [isDrain, setIsDrain] = useState(false);
    const isDrainRef = useRef(isDrain);
    isDrainRef.current = isDrain;
    const [enableAI, setEnableAI] = useState(true);

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
        setToOpen(gameMgrRef.current.toOpen);
        if (!gameMgrRef.current.started) {
            setIsGameOver(true);
            setIsWon(gameMgrRef.current.succeed);
            setHasBest(false);
            setIsSettled(true);
            if (gameMgrRef.current.succeed)
                setRate([0, gameMgrRef.current.allBits]);
            if (isAutoRestartRef.current)
                setTimeout(() => {
                    // just in case the user cancels during the period
                    if (isAutoRestartRef.current)
                        onRestart();
                }, 1500);
        } else {
            if (isDrainRef.current)
                gameMgrRef.current.solve(module.SolvingState.AUTO, false);
            else
                gameMgrRef.current.solve(module.SolvingState.HEUR, false);
            setHasBest(!!gameMgrRef.current.bestBlocks.size());
            setIsSettled(gameMgrRef.current.settled);
            setRate([gameMgrRef.current.bits, gameMgrRef.current.allBits]);
        }
        if (enableAI && isAutoFlagRef.current) {
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
        setHasBest(false);
        if (cancellerRef.current) {
            clearTimeout(cancellerRef.current);
            setCanceller(undefined);
        }
        setMode(null);
        const cfg = module.parse(config);
        module.cache(cfg.width, cfg.height, cfg.totalMines);
        const mgr = new module.GameMgr(cfg.width, cfg.height, cfg.totalMines, cfg.isSNR, cfg, false);
        setGameMgr(mgr);
        setRate([mgr.bits, mgr.allBits]);
        setToOpen(width * height - totalMines);
        if (m === 'auto')
            setTimeout(onAuto, 500);
        else if (m === 'auto-all')
            setTimeout(onAutoAll, 500);
    }

    useEffect(() => {
        onRestart();
        return () => {
            if (gameMgrRef.current) {
                gameMgrRef.current.delete();
                setGameMgr(undefined);
            }
        };
        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, [config]);

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
        if (!gameMgrRef.current.semiAutomaticStep(module.SolvingState.SEMI, true)) {
            if (isDrainRef.current)
                gameMgrRef.current.automaticStep(module.SolvingState.AUTO);
            else
                gameMgrRef.current.automaticStep(module.SolvingState.HEUR);
        }
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
        gameMgrRef.current.automatic(isDrainRef.current);
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

    function onSwitchAI(e) {
        setEnableAI(e.currentTarget.checked);
    }

    function onSwitchDrain(e) {
        setIsDrain(e.currentTarget.checked);
    }

    function roundDigits(v) {
        const av = Math.abs(v);
        if (av >= 0.0001 && av < 100000)
            return '' + Math.round(v);
        const shift = Math.floor(Math.log10(v));
        const scale = Math.pow(10, shift);
        const rv = Math.round(av / scale * 10) / 10;
        return `${Math.sign(v) < 0 ? '-' : ''}${rv}e${shift}`;
    }

    const isReady = gameMgr && gameMgr.totalWidth === width && gameMgr.totalHeight === height;

    return (
        <>
            <Card elevation={Elevation.TWO} className="game">
                <FormGroup label={`${totalFlagged} / ${totalMines} mines flagged`}>
                    <ProgressBar value={totalFlagged / totalMines} stripes={!isGameOver}
                                 intent={(isGameOver && !isWon) ? 'danger' : 'warning'} />
                </FormGroup>
                <Board
                    width={width}
                    height={height}
                    isStarted={isSettled}
                    isGameOver={isGameOver}
                    isWon={isWon}
                    gameMgr={isReady && gameMgr}
                    module={module}
                    flagging={flagging}
                    onProbe={onProbe}
                    onFlag={onFlag}
                    enableAI={enableAI}
                />
                <br />
                <Collapse isOpen={enableAI}>
                    <FormGroup label={`${roundDigits(Math.pow(2, rate[0]))} possible solutions`}>
                        <ProgressBar value={1 - rate[0] / rate[1]} stripes={!isGameOver}
                                     intent={(isGameOver && !isWon) ? 'danger' : 'success'} />
                    </FormGroup>
                </Collapse>
                <FormGroup label={`${toOpen} / ${width * height - totalMines} blocks left`}>
                    <ProgressBar value={1 - toOpen / (width * height - totalMines)}
                                 stripes={!isGameOver}
                                 intent={(isGameOver && !isWon) ? 'danger' : 'primary'} />
                </FormGroup>
            </Card>
            <Card elevation={Elevation.TWO} className="control">
                <h3>Game Control</h3>
                <ControlGroup vertical>
                    <ButtonGroup>
                        <Button disabled={!gameMgr || mode != null} icon="cross"
                                className="growing" text="Close" onClick={onStop} />
                        <Button disabled={!gameMgr || mode != null} rightIcon="refresh"
                                intent={isGameOver ? isWon ? 'success' : 'danger' : undefined}
                                className="growing" text="Restart" onClick={onRestart} />
                    </ButtonGroup>
                </ControlGroup>
                <br />
                <Switch checked={isAutoRestart} onChange={onSwitch}
                        labelElement={'Auto-restart'}
                        innerLabelChecked="on" innerLabel="off"
                        alignIndicator={Alignment.RIGHT} />
                <Switch checked={enableAI} onChange={onSwitchAI}
                        labelElement={'Enable AI'} disabled={mode !== null}
                        innerLabelChecked="on" innerLabel="off"
                        alignIndicator={Alignment.RIGHT} />
                <Collapse isOpen={enableAI} keepChildrenMounted>
                    <h3>AI Control</h3>
                    <pre>{isDrain ? strategy : strategy.replace('-D256', '')}</pre>
                    <Switch checked={isDrain} onChange={onSwitchDrain}
                            labelElement={'Exhaustive'} disabled={mode !== null}
                            innerLabelChecked="PSEQ-D256" innerLabel="PSEQ"
                            alignIndicator={Alignment.RIGHT} />
                    <Switch checked={isAutoFlag} onChange={onSwitchFlag}
                            labelElement={'Auto-flag'}
                            innerLabelChecked="on" innerLabel="off"
                            alignIndicator={Alignment.RIGHT} />
                    <ControlGroup vertical>
                        <ButtonGroup>
                            <Button disabled={!gameMgr || isGameOver || mode !== null}
                                    icon="hand-up" intent="primary" className="growing"
                                    text="Single step" onClick={onStep} />
                        </ButtonGroup>
                    </ControlGroup>
                    <br />
                    <FormGroup label="Speed">
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
                    </FormGroup>
                    <ControlGroup vertical>
                        <ButtonGroup>
                            <Button disabled={!gameMgr || isGameOver || (mode !== null && mode !== 'semi') || !hasBest}
                                    active={mode === 'semi'}
                                    icon="play" intent="success" className="growing"
                                    text="Semi-auto" onClick={onSemi} />
                            <Button disabled={!gameMgr || isGameOver || mode !== null || !hasBest}
                                    icon="fast-forward" intent="success"
                                    onClick={onSemiAll} />
                        </ButtonGroup>
                        <ButtonGroup>
                            <Button disabled={!gameMgr || isGameOver || (mode !== null && mode !== 'auto')}
                                    active={mode === 'auto'}
                                    icon="fast-forward" intent="warning" className="growing"
                                    text="Full-auto" onClick={onAuto} />
                            <Button disabled={!gameMgr || isGameOver || mode !== null}
                                    icon="lightning" intent="warning"
                                    onClick={onAutoAll} />
                        </ButtonGroup>
                    </ControlGroup>
                </Collapse>
            </Card>
        </>
    );
}
