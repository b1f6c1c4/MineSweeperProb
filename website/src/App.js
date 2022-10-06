import React from 'react';
import MineSweeperSolver from './MineSweeperSolver';
import Board from './Board';
import {useEffect, useReducer, useState} from 'react';
import {Button, ButtonGroup, Spinner} from '@blueprintjs/core';
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
    function onUpdate() {
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
    function onSemi() {
        gameMgr.semiAutomatic(module.SolvingState.AUTOMATIC);
        onUpdate();
    }
    function onAuto() {
        gameMgr.automatic();
        onUpdate();
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
                ></Board>)
            : (
                <Spinner intent="primary" />
            )}
            <div>
                <ButtonGroup>
                    <Button disabled={!gameMgr} icon="reset" intent="danger" text="Restart" onClick={onRestart} />
                    <Button disabled={!gameMgr || isGameOver} icon="step-forward" intent="primary" text="Single step" onClick={onStep} />
                    <Button disabled={!gameMgr || isGameOver} icon="play" intent="success" text="Semi-auto" onClick={onSemi} />
                    <Button disabled={!gameMgr || isGameOver} icon="fast-forward" intent="warning" text="Full-auto" onClick={onAuto} />
                </ButtonGroup>
            </div>
        </div>
    );
}

export default App;
