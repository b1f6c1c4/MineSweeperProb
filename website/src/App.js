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
    Spinner,
    Switch
} from "@blueprintjs/core";

const moduleLoader = window.MineSweeperSolver({ locateFile: () => 'MineSweeperSolver.wasm' });

export default function App(props) {
    const [module, setModule] = useState(undefined);
    const [cfg, setCfg] = useState({ text: '(select)' });
    const [isSNR, setIsSNR] = useState(false);
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
            setCfg({text: e.currentTarget.value});
            return;
        }
        setCfg({
            text: e.currentTarget.value,
            width: +m.groups['w'],
            height: +m.groups['h'],
            totalMines: +m.groups['m'],
        });
    }

    function onSwitch(e) {
        setIsSNR(e.currentTarget.checked);
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
        <>
            <Card elevation={Elevation.TWO} className="control">
                <h3>Showcase Playground</h3>
                <FormGroup label="Board" inline>
                    <HTMLSelect value={cfg.text} onChange={onSelect}>
                        <option>(select)</option>
                        <option>8-8-T10</option>
                        <option>9-9-T10</option>
                        <option>16-16-T40</option>
                        <option>30-16-T99</option>
                    </HTMLSelect>
                </FormGroup>
                <Switch checked={isSNR} onChange={onSwitch}
                        labelElement={'Rule'}
                        innerLabelChecked="SNR" innerLabel="SFAR"
                        alignIndicator={Alignment.RIGHT} />
                {!isSNR && (<>
                    <h4>Single First Action Rule</h4>
                    <p>Your first click is guaranteed to be safe.
                        This is the default behavior for the old Microsoft Minesweeper game
                        as well as many competitive Minesweeper games.</p>
                </>)}
                {isSNR && (<>
                    <h4>Single Neighborhood Rule</h4>
                    <p>Your first click is guaranteed to be safe.
                        Furthermore, all immediate neighbors of your first click is also safe.
                        This is the default behavior for newer Microsoft Minesweeper.</p>
                </>)}
                <br />
                <ControlGroup vertical>
                    {module ? (
                        <ButtonGroup>
                            <Button disabled={!cfg.width} icon="play" className="growing"
                                    intent="primary" text="Play" onClick={toggleStart} />
                        </ButtonGroup>
                    ) : (
                        <Spinner intent="primary" />
                    )}
                </ControlGroup>
            </Card>
            <Card elevation={Elevation.THREE} className="intro">
                <h1>Introduction</h1>
                <p></p>
            </Card>
        </>
    );
}
