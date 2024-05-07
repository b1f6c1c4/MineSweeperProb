import React, {useEffect, useReducer, useState} from 'react';
import Game from './Game';
import './App.css';
import {
    Alignment,
    Button, ButtonGroup,
    Card,
    ControlGroup,
    Elevation,
    FormGroup,
    HTMLSelect,
    NumericInput,
    Spinner,
    Switch
} from "@blueprintjs/core";

const moduleLoader = window.MineSweeperSolver({ locateFile: () => 'MineSweeperSolver.wasm' });

export default function App(props) {
    const [module, setModule] = useState(undefined);
    const [cfg, setCfg] = useState({ text: '(select)' });
    const [isSNR, setIsSNR] = useState(false);
    const [isExternal, setIsExternal] = useState(false);
    const [isStarted, toggleStart] = useReducer(x => !x, false);

    useEffect(() => {
        moduleLoader.then((module) => {
            setModule(module);
            module.seed();
        }, console.error);
    }, []);

    function onSelect(e) {
        const m = e.currentTarget.value.match(/^(?<w>[0-9]+)-(?<h>[0-9]+)-T(?<m>[0-9]+)$/);
        if (!m) {
            setCfg({
                text: e.currentTarget.value,
                width: 16,
                height: 16,
                totalMines: 40,
            });
            return;
        }
        setCfg({
            text: e.currentTarget.value,
            width: +m.groups['w'],
            height: +m.groups['h'],
            totalMines: +m.groups['m'],
        });
    }

    function sw(f) {
        return (e) => f(e.currentTarget.checked);
    }

    function onW(v) {
        setCfg({ ...cfg, width: v });
    }

    function onH(v) {
        setCfg({ ...cfg, height: v });
    }

    function onT(v) {
        setCfg({ ...cfg, totalMines: v });
    }

    let strategy = 'FL';
    if (!isSNR)
        strategy += '@[1,1]';
    else if (cfg.width > 20)
        strategy += '@[4,4]';
    else
        strategy += '@[3,3]';
    strategy += '-PSEQ';
    strategy += '-D256';
    let config = strategy + `-${cfg.width}-${cfg.height}-T${cfg.totalMines}`;
    if (isSNR)
        config += '-SNR';
    else
        config += '-SFAR';

    document.getElementById('intro').hidden = isStarted;

    const ready = cfg.text !== '(select)' && cfg.width && cfg.height && cfg.totalMines;

    if (isStarted)
        return (
            <Game
                module={module}
                width={cfg.width}
                height={cfg.height}
                totalMines={cfg.totalMines}
                isSNR={isSNR}
                strategy={strategy}
                config={config}
                onStop={toggleStart}
            />);

    return (
        <Card elevation={Elevation.TWO} className="control">
            <h3>Showcase &amp; Playground</h3>
            <Switch checked={isExternal} onChange={sw(setIsExternal)}
                labelElement={'External'}
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
            {isExternal && (<>
                <h4>External Mode</h4>
                <p>Choose this option if you wish to analyze an existing Minesweeper game, instead of starting a new one.</p>
                </>)}
            {!isExternal && (
                <Switch checked={isSNR} onChange={sw(setIsSNR)}
                    labelElement={'Rule'}
                    innerLabelChecked="SNR" innerLabel="SFAR"
                    alignIndicator={Alignment.RIGHT} />
                )}
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
            <br />
            <ControlGroup vertical>
                {module ? (
                    <ButtonGroup>
                        <Button disabled={!ready} icon="play" className="growing"
                        intent="primary" text="Play" onClick={toggleStart} />
                    </ButtonGroup>
                ) : (
                    <Spinner intent="primary" />
                )}
            </ControlGroup>
        </Card>
    );
}
