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
        isExternal,
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
    const [history, setHistory] = useState(undefined);
    const historyRef = useRef(history);
    historyRef.current = history;

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

    // undefined or null: mutable; 'X': immutable;
    // 0~8: degree to be assigned; 'M': must be mine, 'E': must no mine
    const [overlay, setOverlay] = useState([]);
    const overlayRef = useRef(overlay);
    overlayRef.current = overlay;

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
    function onUpdate(json, isRedo) {
        if (json) {
            const { f, o, fake } = JSON.parse(json);
            if (fake)
                historyRef.current.drop();
            if (isExternal) {
                setOverlay(o);
                setEnableAI(isRedo);
            } else {
                setFlagging(f);
            }
        }
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
            setIsGameOver(false);
            setIsWon(false);
            if (isDrainRef.current)
                gameMgrRef.current.solve(module.SolvingState.AUTO, false);
            else
                gameMgrRef.current.solve(module.SolvingState.HEUR, false);
            setHasBest(!!gameMgrRef.current.bestBlocks.size());
            setIsSettled(gameMgrRef.current.settled);
            setRate([gameMgrRef.current.bits, gameMgrRef.current.allBits]);
        }
        if (enableAI && isAutoFlagRef.current && !json) {
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
        if (gameMgrRef.current)
            gameMgrRef.current.delete();
        if (historyRef.current)
            historyRef.current.delete();
        const m = modeRef.current;
        if (isExternal)
            setEnableAI(true);
        setIsSettled(false);
        setIsGameOver(false);
        setIsWon(false);
        setOverlay([]);
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
        const mgr = isExternal
            ? new module.GameMgr(cfg.width, cfg.height, cfg.totalMines, cfg)
            : new module.GameMgr(cfg.width, cfg.height, cfg.totalMines, cfg.isSNR, cfg, false);
        const his = new module.History();
        setGameMgr(mgr);
        setHistory(his);
        setRate([mgr.bits, mgr.allBits]);
        setToOpen(width * height - totalMines);
        if (m === 'auto')
            setTimeout(onAuto, 500);
        else if (m === 'auto-all')
            setTimeout(onAutoAll, 500);
    }

    function onUndo() {
        if (isExternal) {
            if (!enableAI) { // "Clear"
                setOverlay(overlay.map((v) => v === 'X' ? v : undefined));
                setEnableAI(true);
            } else {
                if (!history.redoable)
                    push(undefined, true);
                onUpdate(history.undo(gameMgr));
            }
        } else {
            onUpdate(history.undo(gameMgr));
        }
    }

    function onRedo() {
        onUpdate(history.redo(gameMgr), true);
    }

    useEffect(() => {
        onRestart();
        return () => {
            if (gameMgrRef.current) {
                gameMgrRef.current.delete();
                historyRef.current.delete();
                setGameMgr(undefined);
                setHistory(undefined);
            }
        };
        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, [config]);

    function onRotate(row, col) {
        const ov = [...overlay];
        const id = col * height + row;
        switch (overlay[id]) {
            case 'X':
                return;
            case null:
            case undefined:
            case 'E':
            case 'M':
            case 0:
                ov[id] = 1;
                break;
            case 8:
                ov[id] = 1;
                break;
            default:
                ov[id]++;
                break;
        }
        setOverlay(ov);
        setEnableAI(false);
    }

    function onUnRotate(row, col) {
        const ov = [...overlay];
        const id = col * height + row;
        switch (ov[id]) {
            case 'X':
                return;
            case null:
            case undefined:
                ov[id] = 0;
                break;
            case 0:
                ov[id] = 'M';
                break;
            case 'M':
                ov[id] = 'E';
                break;
            case 'E':
            default:
                delete ov[id];
                break;
        }
        setOverlay(ov);
        setEnableAI(false);
    }

    // could be inside setTimeout
    function push(f, fake) {
        historyRef.current.push(gameMgrRef.current, JSON.stringify({
            f: f || flaggingRef.current,
            o: overlayRef.current,
            fake,
        }));
    }

    function onProbe(row, col) {
        if (flagging[col * height + row])
            return;
        if (gameMgr.openBlock(col, row)) {
            onUpdate();
            push();
        }
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
        push(f);
    }

    // could be inside setTimeout
    function onStep() {
        const ss = isDrainRef.current ? module.SolvingState.AUTO : module.SolvingState.HEUR;
        if (isExternal) {
            push();
            overlayRef.current.forEach((v, i) => {
                const row = i % height;
                const col = (i - row) / height;
                switch (v) {
                    case 'M':
                        gameMgrRef.current.setBlockMine(col, row, true);
                        break;
                    case 'E':
                        gameMgrRef.current.setBlockMine(col, row, false);
                        break;
                    case 'X':
                    case null:
                    case undefined:
                        break;
                    default:
                        gameMgrRef.current.setBlockDegree(col, row, v);
                        break;
                }
            });
            setOverlay(overlayRef.current.map((v) => (v === undefined || v == null) ? v : 'X'));
            gameMgrRef.current.solve(ss, false);
            setEnableAI(true);
            onUpdate();
        } else {
            if (!gameMgrRef.current.semiAutomaticStep(module.SolvingState.SEMI, true)) {
                gameMgrRef.current.automaticStep(ss);
            }
            onUpdate();
            push();
        }
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
        push();
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
        push();
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
                {!isExternal && (
                    <FormGroup label={`${totalFlagged} / ${totalMines} mines flagged`}>
                        <ProgressBar value={totalFlagged / totalMines} stripes={!isGameOver}
                                     intent={(isGameOver && !isWon) ? 'danger' : 'warning'} />
                    </FormGroup>
                    )}
                <Board
                    width={width}
                    height={height}
                    isExternal={isExternal}
                    isStarted={isSettled}
                    isGameOver={isGameOver}
                    isWon={isWon}
                    gameMgr={isReady && gameMgr}
                    module={module}
                    overlay={overlay}
                    flagging={flagging}
                    onProbe={isExternal ? onRotate : onProbe}
                    onFlag={isExternal ? onUnRotate : onFlag}
                    enableAI={enableAI}
                />
                <br />
                <Collapse isOpen={enableAI}>
                    <FormGroup label={`${roundDigits(Math.pow(2, rate[0]))} possible solutions`}>
                        <ProgressBar value={1 - rate[0] / rate[1]} stripes={!isGameOver}
                                     intent={(isGameOver && !isWon) ? 'danger' : 'success'} />
                    </FormGroup>
                </Collapse>
                {!isExternal && (
                    <FormGroup label={`${toOpen} / ${width * height - totalMines} blocks left`}>
                        <ProgressBar value={1 - toOpen / (width * height - totalMines)}
                                     stripes={!isGameOver}
                                     intent={(isGameOver && !isWon) ? 'danger' : 'primary'} />
                    </FormGroup>
                    )}
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
                    <ButtonGroup>
                        <Button disabled={!gameMgr || mode != null || !history || !(isExternal ? (!enableAI || history.xundoable) : history.undoable)} icon="undo"
                                className="growing" text={isExternal && !enableAI ? 'Clear' : 'Undo'} onClick={onUndo} />
                        <Button disabled={!gameMgr || mode != null || !history || !history.redoable} rightIcon="redo"
                                className="growing" text="Redo" onClick={onRedo} />
                    </ButtonGroup>
                </ControlGroup>
                {!isExternal && (<>
                    <br />
                    <Switch checked={isAutoRestart} onChange={onSwitch}
                            labelElement={'Auto-restart'}
                            innerLabelChecked="on" innerLabel="off"
                            alignIndicator={Alignment.RIGHT} />
                    <Switch checked={enableAI} onChange={onSwitchAI}
                            labelElement={'Enable AI'} disabled={mode !== null}
                            innerLabelChecked="on" innerLabel="off"
                            alignIndicator={Alignment.RIGHT} />
                    </>)}
                <Collapse isOpen={enableAI || isExternal} keepChildrenMounted>
                    <h3>AI Control</h3>
                    <pre>{isDrain ? strategy : strategy.replace('-D256', '')}</pre>
                    <Switch checked={isDrain} onChange={onSwitchDrain}
                            labelElement={'Exhaustive'} disabled={mode !== null}
                            innerLabelChecked="PSEQ-D256" innerLabel="PSEQ"
                            alignIndicator={Alignment.RIGHT} />
                    {!isExternal && (
                        <Switch checked={isAutoFlag} onChange={onSwitchFlag}
                                labelElement={'Auto-flag'}
                                innerLabelChecked="on" innerLabel="off"
                                alignIndicator={Alignment.RIGHT} />
                        )}
                    <ControlGroup vertical>
                        <ButtonGroup>
                            <Button disabled={!gameMgr || isGameOver || mode !== null || (isExternal && enableAI)}
                                    icon="hand-up" intent="primary" className="growing"
                                    text={isExternal ? 'Solve' : 'Single step'}
                                    onClick={onStep} />
                        </ButtonGroup>
                    </ControlGroup>
                    {!isExternal && (<>
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
                        </>)}
                </Collapse>
                {isExternal && (<>
                    <h4>External Mode</h4>
                    <p>Use left mouse click to indicate the degree of each block. Use right click to toggle between 'e' (assumed not to have mine) and 'M' (assumed to have mine.) Once you click Solve, you <strong>cannot</strong> edit existing degrees.</p>
                    </>)}
            </Card>
        </>
    );
}
