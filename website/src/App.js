import React from 'react';
import MineSweeperSolver from './MineSweeperSolver';
import Board from './Board';
import {useEffect, useReducer, useState} from 'react';
import {Button, ButtonGroup, ProgressBar, Slider, Spinner} from '@blueprintjs/core';
import './App.css';

const moduleLoader = MineSweeperSolver({ locateFile: () => 'MineSweeperSolver.wasm' });

function App(props) {
    const {
        width,
        height,
    } = props;
    const [module, setModule] = useState(undefined);
    const [gameMgr, setGameMgr] = useState(undefined);
    useEffect(() => {
        moduleLoader.then((module) => {
            setModule(module);
            module.seed();
            const cfg = module.parse('FL@[4,4]-PSEQ-30-16-T99-SNR');
            module.cache(cfg.width, cfg.height, cfg.totalMines);
            const mgr = new module.GameMgr(cfg.width, cfg.height, cfg.totalMines, cfg.isSNR, cfg, false);
            setGameMgr(mgr);
        }, console.error);
    }, [width, height]);

    const [, forceUpdate] = useReducer(x => x + 1, 0);
    const [isSettled, setIsSettled] = useState(false);
    const [isGameOver, setIsGameOver] = useState(false);
    const [isWon, setIsWon] = useState(false);
    const [rate, setRate] = useState(0);
    function onUpdate() {
        setRate(1 - gameMgr.bits / gameMgr.allBits);
        setIsSettled(gameMgr.settled);
        if (!gameMgr.started) {
            setIsGameOver(true);
            setIsWon(gameMgr.succeed);
        } else {
            gameMgr.solve(module.SolvingState.AUTOMATIC, false);
        }
        forceUpdate();
    }

    function onRestart() {
        setIsSettled(false);
        setIsGameOver(false);
        setIsWon(false);
        setFlagging([]);
        setRate(0);
        setMode(null);
        if (canceller) {
            clearTimeout(canceller);
            setCanceller(undefined);
        }
        const cfg = module.parse('FL@[4,4]-PSEQ-30-16-T99-SNR');
        module.cache(cfg.width, cfg.height, cfg.totalMines);
        const mgr = new module.GameMgr(cfg.width, cfg.height, cfg.totalMines, cfg.isSNR, cfg, false);
        setGameMgr(mgr);
    }

    function onProbe(row, col) {
        gameMgr.openBlock(col, row);
        onUpdate();
    }
    const [flagging, setFlagging] = useState([]);
    function onFlag(row, col) {
        const f = [...flagging];
        f[row * width + col] ^= true;
        setFlagging(f);
    }

    function onStep() {
        if (!gameMgr.semiAutomaticStep(module.SolvingState.AUTOMATIC, true))
            gameMgr.automaticStep(module.SolvingState.AUTOMATIC);
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
                const next = gameMgr.semiAutomaticStep(module.SolvingState.AUTOMATIC, true);
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
                if (!gameMgr.semiAutomaticStep(module.SolvingState.AUTOMATIC, true))
                    gameMgr.automaticStep(module.SolvingState.AUTOMATIC);
                onUpdate();
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
        <div className="App">
            {gameMgr ? (
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
                />)
            : (
                <Spinner intent="primary" />
            )}
            <div>
                <ProgressBar value={rate} stripes={!isGameOver}
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

export default App;
