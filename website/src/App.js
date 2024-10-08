import React, {useEffect, useState} from 'react';
import Game from './Game';
import './App.css';
import {
    Alignment,
    Button, ButtonGroup,
    Card,
    ControlGroup,
    Elevation,
    FileInput,
    FormGroup,
    HTMLSelect,
    NumericInput,
    Spinner,
    Switch,
    Tooltip,
} from "@blueprintjs/core";

const moduleLoader = window.MineSweeperSolver({ locateFile: () => 'MineSweeperSolver.wasm' });

export default function App(props) {
    const [module, setModule] = useState(undefined);
    const [cfg, setCfg] = useState({ text: '(select)' });
    const [isSNR, setIsSNR] = useState(false);
    const [isExternal, setIsExternal] = useState(false);
    const [isStarted, setStarted] = useState(false);
    const [loaded, setLoaded] = useState(undefined);
    const [isManual, setIsManual] = useState(false);

    useEffect(() => {
        const r = (e) => {
            if (!e.state) {
                setStarted(false);
                setLoaded(undefined);
            } else {
                setCfg(e.state.cfg);
                setIsSNR(e.state.isSNR);
                setIsExternal(e.state.isExternal);
                setLoaded(e.state.loaded);
                setIsManual(e.state.isManual);
                setStarted(e.state.isStarted);
            }
        };
        moduleLoader.then((module) => {
            setModule(module);
            module.seed();
            r(window.history);
            window.addEventListener('popstate', r);
        }, console.error);
        return () => window.removeEventListener('popstate', r);
    }, []);

    function onSelect(e) {
        const m = e.currentTarget.value.match(/^(?<w>[0-9]+)-(?<h>[0-9]+)-T(?<m>[0-9]+)$/);
        if (!m) {
            onSetCfg({
                text: e.currentTarget.value,
                width: 16,
                height: 16,
                totalMines: 40,
            });
            return;
        }
        onSetCfg({
            text: e.currentTarget.value,
            width: +m.groups['w'],
            height: +m.groups['h'],
            totalMines: +m.groups['m'],
        });
    }

    function onSetCfg(c) {
        setCfg(c);
        window.history.replaceState({
            isStarted: false,
            cfg: c,
            isSNR,
            isExternal,
            loaded,
        }, '', '/');
    }

    function onSetExt(e) {
        const c = e.currentTarget.checked;
        setIsExternal(c);
        window.history.replaceState({
            isStarted: false,
            cfg,
            isSNR,
            isExternal: c,
            loaded,
        }, '', '/');
    }

    function onSetSNR(e) {
        const c = e.currentTarget.checked;
        setIsSNR(c);
        window.history.replaceState({
            isStarted: false,
            cfg,
            isSNR: c,
            isExternal,
            loaded,
        }, '', '/');
    }

    function onStart() {
        window.history.pushState({
            isStarted: true,
            cfg,
            isSNR,
            isExternal,
            loaded,
        }, '', '#game');
        setStarted(true);
    }

    function onStop() {
        window.history.pushState({
            isStarted: false,
            cfg,
            isSNR,
            isExternal,
        }, '', '/');
        setStarted(false);
        setLoaded(undefined);
    }

    function onPersist(g) {
        window.history.replaceState({
            isStarted: true,
            cfg,
            isSNR,
            isExternal,
            loaded: g,
            isManual,
        }, '', '#game');
    }

    function onW(v) {
        onSetCfg({ ...cfg, width: v });
    }

    function onH(v) {
        onSetCfg({ ...cfg, height: v });
    }

    function onT(v) {
        onSetCfg({ ...cfg, totalMines: v });
    }

    async function onLoad(e) {
        setLoaded(JSON.parse(await e.target.files[0].text()));
        setIsManual(true);
        onStart();
    }

    let strategy = 'FL';
    if (!isSNR)
        strategy += '@[1,1]';
    else if (cfg.width > 20)
        strategy += '@[4,4]';
    else
        strategy += '@[3,3]';
    strategy += '-2PSEQ';
    strategy += '-D256';
    let config = strategy + `-${cfg.width}-${cfg.height}-T${cfg.totalMines}`;
    if (isSNR)
        config += '-SNR';
    else
        config += '-SFAR';

    document.getElementById('intro').hidden = isStarted;

    const ready = cfg.text !== '(select)' && cfg.width && cfg.height && cfg.totalMines;

    if (isStarted) {
        if (isExternal && cfg.text === '(dynamic)') {
            return (
                <pre>TODO</pre>
            );
        } else if (loaded) {
            return (
                <Game
                    module={module}
                    isManual={isManual}
                    {...loaded}
                    onStop={onStop}
                    onPersist={onPersist}
                />);
        } else {
            return (
                <Game
                    module={module}
                    isManual={isManual}
                    isExternal={isExternal}
                    width={cfg.width}
                    height={cfg.height}
                    totalMines={cfg.totalMines}
                    strategy={strategy}
                    config={config}
                    onStop={onStop}
                    onPersist={onPersist}
                />);
        }
    }

    return (
        <Card elevation={Elevation.TWO} className="control app">
            <h3>Showcase &amp; Playground</h3>
            <Switch checked={isExternal} onChange={onSetExt}
                labelElement="Mode"
                innerLabelChecked="Analyze" innerLabel="New Game"
                alignIndicator={Alignment.RIGHT} />
            <FormGroup label="Board" inline>
                <HTMLSelect value={cfg.text} onChange={onSelect}>
                    {isExternal ? (
                        <option>(dynamic)</option>
                    ) : (
                        <option>(select)</option>
                    )}
                    <option>8-8-T10</option>
                    <option>9-9-T10</option>
                    <option>16-16-T40</option>
                    <option>30-16-T99</option>
                    <option>(custom)</option>
                </HTMLSelect>
            </FormGroup>
            {cfg.text === '(custom)' && (<>
                <FormGroup label="Width" inline>
                    <NumericInput onValueChange={onW} min={2} max={99}
                        fill small stepSize={1} minorStepSize={null}
                        value={cfg.width} />
                </FormGroup>
                <FormGroup label="Height" inline>
                    <NumericInput onValueChange={onH} min={2} max={99}
                        fill small stepSize={1} minorStepSize={null}
                        value={cfg.height} />
                </FormGroup>
                <FormGroup label="Mines" inline>
                    <NumericInput onValueChange={onT} min={1}
                        max={cfg.width * cfg.height}
                        fill small stepSize={1} minorStepSize={null}
                        value={cfg.totalMines} />
                </FormGroup>
                </>)}
            {!isExternal && (
                <Switch checked={isSNR} onChange={onSetSNR}
                    labelElement={'Rule'}
                    innerLabelChecked="SNR" innerLabel="SFAR"
                    alignIndicator={Alignment.RIGHT} />
                )}
            <div className="description">
                {isExternal && (<>
                    <h4>External Mode</h4>
                    <p>Choose this option if you wish to analyze an existing Minesweeper game, instead of starting a new one.</p>
                    </>)}
                {!isExternal && !isSNR && (<>
                    <h4>Single First Action Rule</h4>
                    <p>Your first click is guaranteed to be safe.
                    This is the default behavior for the old Microsoft Minesweeper game
                    as well as many competitive Minesweeper games.</p>
                    </>)}
                {!isExternal && isSNR && (<>
                    <h4>Single Neighborhood Rule</h4>
                    <p>Your first click is guaranteed to be safe.
                    Furthermore, all immediate neighbors of your first click is also safe.
                    This is the default behavior for newer Microsoft Minesweeper.</p>
                    </>)}
            </div>
            <br />
            <ControlGroup vertical>
                {module ? (
                    <ButtonGroup>
                        <Tooltip content={ready ? '' : 'Select board first'}>
                            <Button disabled={!ready} icon="play" className="growing"
                                intent="primary" text="New Game" onClick={onStart} />
                        </Tooltip>
                    </ButtonGroup>
                ) : (
                    <Spinner intent="primary" />
                )}
            </ControlGroup>
            <h4>Load saved game</h4>
            <FileInput text="Choose file..." onInputChange={onLoad}
                fill inputProps={{ accept: 'application/json' }} />
        </Card>
    );
}
