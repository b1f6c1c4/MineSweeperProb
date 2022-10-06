import './App.css';
import MineSweeperSolver from './MineSweeperSolver';
import Board from './Board';
import {useEffect, useReducer, useState} from 'react';

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
    const [isStarted, setIsStarted] = useState(false);
    const [isGameOver, setIsGameOver] = useState(false);
    const [isWon, setIsWon] = useState(false);
    function onUpdate() {
        if (!gameMgr.started) {
            setIsGameOver(true);
            setIsWon(gameMgr.succeed);
        } else {
            gameMgr.solve(module.SolvingState.AUTOMATIC, false);
        }
        forceUpdate();
    }

    function onProbe(row, col) {
        gameMgr.openBlock(col, row);
        setIsStarted(true);
        gameMgr.semiAutomatic(module.SolvingState.AUTOMATIC);
        onUpdate();
    }
    const [flagging, setFlagging] = useState([]);
    function onFlag(row, col) {
        const f = [...flagging];
        f[row * width + col] ^= true;
        setFlagging(f);
    }

    return (
        <div className="App">
            {gameMgr ? (
                <Board
                    width={width}
                    height={height}
                    isStarted={isStarted}
                    isGameOver={isGameOver}
                    isWon={isWon}
                    gameMgr={gameMgr}
                    module={module}
                    flagging={flagging}
                    onProbe={onProbe}
                    onFlag={onFlag}
                ></Board>)
            : 'Loading'}
        </div>
    );
}

export default App;
