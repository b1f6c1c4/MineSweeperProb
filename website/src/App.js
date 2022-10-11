import React, {useEffect, useState} from 'react';
import Game from './Game';
import './App.css';
import {HTMLSelect, Spinner, Switch} from "@blueprintjs/core";

const moduleLoader = window.Module({ locateFile: () => 'MineSweeperSolver.wasm' });

export default function App(props) {
    const [module, setModule] = useState(undefined);
    const [cfg, setCfg] = useState({ text: '(select)' });
    const [isSNR, setIsSNR] = useState(false);
    const [isDrain, setIsDrain] = useState(false);

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

    function onSwitchDrain(e) {
        setIsDrain(e.currentTarget.checked);
    }

    let strategy = 'FL';
    if (!isSNR)
        strategy += '@[1,1]';
    else if (cfg.width > 20)
        strategy += '@[4,4]';
    else
        strategy += '@[3,3]';
    strategy += '-PSEQ';
    if (isDrain)
        strategy += '-D256';
    strategy += `-${cfg.width}-${cfg.height}-T${cfg.totalMines}`;
    if (isSNR)
        strategy += '-SNR';
    else
        strategy += '-SFAR';

    return (
        <div>
            <HTMLSelect value={cfg.text} onChange={onSelect}>
                <option>(select)</option>
                <option>8-8-T10</option>
                <option>9-9-T10</option>
                <option>16-16-T40</option>
                <option>30-16-T99</option>
            </HTMLSelect>
            <Switch checked={isSNR} onChange={onSwitch}
                    labelElement={'Safe First Action / Safe Neighbor Rule'}
                    innerLabelChecked="SNR" innerLabel="SFAR" />
            <Switch checked={isDrain} onChange={onSwitchDrain}
                    labelElement={'Enable D256'}
                    innerLabelChecked="PSEQ-D256" innerLabel="PSEQ" />
            {!module ? (
                <Spinner intent="primary" />
            ) : cfg.width && (
                <Game
                    module={module}
                    width={cfg.width}
                    height={cfg.height}
                    totalMines={cfg.totalMines}
                    isSNR={isSNR}
                    strategy={strategy}
                />)}
        </div>
    );
}
