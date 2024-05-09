import React, {useEffect, useReducer, useRef, useState} from 'react';
import Board from './Board';
import Control from './Control';
import {
    Alert,
    Card,
    Collapse,
    Elevation,
    FormGroup,
    ProgressBar,
} from '@blueprintjs/core';

export default function Game(props) {
    const {
        module,
        isManual,
        isExternal,
        width,
        height,
        totalMines,
        strategy,
        config,
        loadedGame,
        loadedFlags,
        loadedOverlay,
        onStop,
        onPersist,
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
    const [drainAlert, setDrainAlert] = useState(false);
    const [isDraining, setIsDraining] = useState(false);
    const isDrainingRef = useRef(isDraining);
    isDrainingRef.current = isDraining;
    const [isDrain, setIsDrain] = useState(false);
    const isDrainRef = useRef(isDrain);
    isDrainRef.current = isDrain;
    const [enableAI, setEnableAI] = useState(true);
    const [isStepHover, setIsStepHover] = useState(false);

    const [, forceUpdate] = useReducer(x => x + 1, 0);
    const [isSettled, setIsSettled] = useState(false);
    const [isGameOver, setIsGameOver] = useState(false);
    const [isWon, setIsWon] = useState(false);
    const [rate, setRate] = useState([1, 1]);
    const [toOpen, setToOpen] = useState(width * height - totalMines);
    const [hasBest, setHasBest] = useState(false);
    const [isSearching, setIsSearching] = useState(false);

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
    const cancellerRef = useRef(undefined);

    // could be inside setTimeout
    function onUpdate(json, isRedo, noSolve) {
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
            gameMgrRef.current.solve(module.SolvingState.HEUR, false);
        }
        setToOpen(gameMgrRef.current.toOpen);
        if (!gameMgrRef.current.started) {
            setIsGameOver(true);
            setIsWon(gameMgrRef.current.succeed);
            setHasBest(false);
            setIsSettled(true);
            if (isExternal) {
                setRate([null, null]);
            } else {
                if (gameMgrRef.current.succeed)
                    setRate([0, gameMgrRef.current.allBits]);
                if (isAutoRestartRef.current)
                    setTimeout(() => {
                        // just in case the user cancels during the period
                        if (isAutoRestartRef.current)
                            onRestart();
                    }, 1500);
            }
        } else {
            setIsGameOver(false);
            setIsWon(false);
            if (!noSolve || isDrainingRef.current) {
                if (isDrainRef.current)
                    gameMgrRef.current.solve(module.SolvingState.AUTO, false);
                else
                    gameMgrRef.current.solve(module.SolvingState.HEUR, false);
            }
            setHasBest(!!gameMgrRef.current.bestBlocks.size());
            setIsSettled(gameMgrRef.current.settled);
            setRate([gameMgrRef.current.bits, gameMgrRef.current.allBits]);
        }
        if (!json) {
            const af = enableAI && isAutoFlagRef.current;
            const next = [...flaggingRef.current];
            let tf = totalFlaggedRef.current;
            for (let i = 0; i < height; i++)
                for (let j = 0; j < width; j++) {
                    if (af && gameMgrRef.current.inferredStatusOf(j, i) === module.BlockStatus.MINE)
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
    function onRestart(isFirst) {
        if (isFirst !== true) // invoked with event => false
            isFirst = false;
        if (isManual) // always load the same
            isFirst = true;

        if (gameMgrRef.current)
            gameMgrRef.current.delete();
        if (historyRef.current)
            historyRef.current.delete();
        const m = modeRef.current;
        if (isExternal)
            setEnableAI(true);
        setIsSettled(!!loadedGame && isFirst);
        setIsDrain(false);
        setIsDraining(false);
        setIsGameOver(false);
        setIsWon(false);
        setOverlay((isFirst && loadedOverlay) || []);
        setFlagging((isFirst && loadedFlags) || []);
        setTotalFlagged(isFirst ? (loadedFlags ?? []).reduce((a,v) => v ? ++a : a, 0) : 0);
        setHasBest(false);
        if (cancellerRef.current) {
            clearTimeout(cancellerRef.current);
            cancellerRef.current = undefined;
        }
        setMode(null);
        const cfg = module.parse(config);
        module.cache(cfg.width, cfg.height, cfg.totalMines);
        let mgr;
        if (isFirst && loadedGame) {
            mgr = module.importGame(loadedGame, cfg);
            setTimeout(onUpdate, 10);
        } else if (isExternal) {
            mgr = new module.GameMgr(cfg.width, cfg.height, cfg.totalMines, cfg);
        } else {
            mgr = new module.GameMgr(cfg.width, cfg.height, cfg.totalMines, cfg.isSNR, cfg, false);
        }
        const his = new module.History(cfg);
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
        onRestart(true);
        return () => {
            if (gameMgrRef.current) {
                gameMgrRef.current.delete();
                historyRef.current.delete();
                setGameMgr(undefined);
                setHistory(undefined);
            }
        };
        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, [config, loadedGame, loadedFlags, loadedOverlay]);

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

    function onSearching() {
        setIsSearching(!isSearching)
    }

    // could be inside setTimeout
    function push(f, fake) {
        if (!(isManual && loadedGame))
            onPersist(persist());
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
        if (gameMgr.blockPropertyOf(col, row).isOpen)
            return;
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
            if (!gameMgrRef.current.semiAutomaticStep(module.SolvingState.SEMI, true, true)) {
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
                cancellerRef.current = undefined;
                const next = gameMgrRef.current.semiAutomaticStep(module.SolvingState.SEMI, true, true);
                push();
                onUpdate();
                if (next)
                    cancellerRef.current = setTimeout(foo, Math.pow(10, 3 + speedRef.current));
                else
                    setMode(null);
            };
            foo();
        } else {
            clearTimeout(cancellerRef.current);
            cancellerRef.current = undefined;
            setMode(null);
        }
    }

    function onSemiEvery() {
        gameMgr.semiAutomaticStep(module.SolvingState.SEMI, false, false);
        onUpdate();
        push();
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
                cancellerRef.current = undefined;
                onStep();
                if (gameMgrRef.current.started)
                    cancellerRef.current = setTimeout(foo, Math.pow(10, 3 + speedRef.current));
                else if (!isAutoRestartRef.current)
                    setMode(null);
            };
            foo();
        } else {
            clearTimeout(cancellerRef.current);
            cancellerRef.current = undefined;
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

    function onCloseDrainAlert() {
        setDrainAlert(false);
    }

    function onDrainAlert() {
        const workload = Math.pow(gameMgr.bits, 1.5) * toOpen;
        if (workload <= 50)
            onDrain();
        else if (workload <= 100)
            setDrainAlert('less than a minute');
        else if (workload <= 250)
            setDrainAlert('roughly a minute');
        else
            setDrainAlert('several minutes');
    }

    // could be inside setTimeout
    function progressDrain() {
        const [x, a] = isDrainingRef.current;
        if (gameMgrRef.current.makeDrainerProgress()) {
            setIsDraining([x + 1, a]);
            setTimeout(progressDrain, 10);
        } else {
            setIsDraining(undefined);
            setIsDrain(true);
            setTimeout(onUpdate, 10);
        }
    }

    function onDrain() {
        push();
        setDrainAlert(false);
        gameMgr.enableDrainer(false);
        onUpdate(undefined, false, true);
        setIsDraining([0, gameMgr.drainerSteps]);
        setTimeout(progressDrain, 50);
    }

    function renderLabel(v) {
        const y = Math.pow(10, -v);
        const s = Math.pow(10, Math.ceil(v) + 1);
        return `${Math.round(y * s) / s}x`;
    }

    function getValue(e, v) {
        if ('checked' in e.currentTarget)
            return e.currentTarget.checked;
        return !v;
    }

    function onSwitchRestart(e) {
        const v = getValue(e, isAutoRestart);
        setIsAutoRestart(v);
        if (isGameOver && !v)
            setMode(null);
    }

    function onSwitchFlag(e) {
        setIsAutoFlag(getValue(e, isAutoFlag));
    }

    function onSwitchAI(e) {
        setEnableAI(getValue(e, enableAI));
    }

    function persist() {
        return {
            isExternal,
            width,
            height,
            totalMines,
            strategy,
            config,
            loadedGame: module.exportGame(gameMgr),
            loadedFlags: flagging,
            loadedOverlay: overlay,
        };
    }

    function onDownload() {
        const blob = new Blob([JSON.stringify(persist())], { type: 'application/json' });
        const link = document.createElement('a');

        link.download = 'game.json';
        link.href = window.URL.createObjectURL(blob);
        link.dataset.downloadurl = [
            'application/json',
            link.download,
            link.href,
        ].join(":");

        const evt = new MouseEvent('click', {
            view: window,
            bubbles: true,
            cancelable: true,
        });

        link.dispatchEvent(evt);
        link.remove()
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
                    isSearching={isSearching}
                    isStarted={isSettled}
                    isGameOver={isGameOver}
                    isDrain={isDrain}
                    isWon={isWon}
                    gameMgr={isReady && gameMgr}
                    module={module}
                    overlay={overlay}
                    flagging={flagging}
                    onProbe={isExternal ? onRotate : onProbe}
                    onFlag={isExternal ? onUnRotate : onFlag}
                    enableAI={enableAI}
                    isLocked={isDraining || mode !== null}
                    isAuto={mode !== null || (isStepHover && isSettled)}
                />
                {enableAI && (
                    <FormGroup label={rate[0] === null ? 'INFEASIBLE - Click undo and retry' : `${roundDigits(Math.pow(2, rate[0]))} possible solutions`}>
                        <ProgressBar value={1 - rate[0] / rate[1]} stripes={!isGameOver}
                                     intent={(isGameOver && !isWon) ? 'danger' : 'success'} />
                    </FormGroup>
                    )}
                {!isExternal && (
                    <FormGroup label={`${toOpen} / ${width * height - totalMines} blocks left`}>
                        <ProgressBar value={1 - toOpen / (width * height - totalMines)}
                                     stripes={!isGameOver}
                                     intent={(isGameOver && !isWon) ? 'danger' : 'primary'} />
                    </FormGroup>
                    )}
            </Card>
            <Control {...{
                enableAI,
                gameMgr,
                hasBest,
                history,
                isAutoFlag,
                isAutoRestart,
                isDrain,
                isDraining,
                isExternal,
                isGameOver,
                isManual: isManual && loadedGame,
                isReady,
                isSearching,
                isSettled,
                isWon,
                mode,
                onAuto,
                onAutoAll,
                onDownload,
                onDrainAlert,
                onRedo,
                onRestart,
                onSearching,
                onSemi,
                onSemiAll,
                onSemiEvery,
                onStep,
                onStop,
                onSwitchAI,
                onSwitchFlag,
                onSwitchRestart,
                onUndo,
                rate,
                renderLabel,
                setIsStepHover,
                setSpeed,
                speed,
                width,
            }} />
            <Alert icon="layout-balloon" intent="danger" isOpen={drainAlert}
                canEscapeKeyCancel canOutsideClickCancel
                cancelButtonText="Cancel" confirmButtonText="Proceed"
                onCancel={onCloseDrainAlert} onConfirm={onDrain}>
                <p>It may take {drainAlert} to compute - do you wish to proceed?</p>
            </Alert>
        </>
    );
}
