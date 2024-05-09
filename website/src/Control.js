import React, {useState, useEffect} from 'react';
import {
    Alignment,
    Button,
    ButtonGroup,
    Card,
    Collapse,
    ControlGroup,
    Elevation,
    FormGroup,
    ProgressBar,
    Slider,
    Switch,
    Tooltip,
} from '@blueprintjs/core';

export default function Control(props) {
    const {
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
        isManual,
        isReady,
        isSettled,
        isWon,
        mode,
        onAuto,
        onAutoAll,
        onDownload,
        onDrainAlert,
        onRedo,
        onRestart,
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
    } = props;

    function getDim() {
        const { innerWidth: width, innerHeight: height } = window;
        return {
            width,
            height
        };
    }
    const [dim, setDim] = useState(getDim());
    useEffect(() => {
        const r = () => { setDim(getDim()); }
        window.addEventListener('resize', r);
        return () => window.removeEventListener('resize', r);
    }, []);

    function onSpeedUp() {
        if (speed > -2)
            setSpeed(speed - 0.2);
    }

    function onSpeedDown() {
        if (speed < +2)
            setSpeed(speed + 0.2);
    }

    const drainable = isReady && !isGameOver && rate[0] <= 8 && !isDrain;

    const disabledGeneral = !gameMgr || mode != null;
    const disabledUndo = !gameMgr || mode != null || !history || !(isExternal ? (!enableAI || history.xundoable) : history.undoable);
    const disabledSave = !gameMgr || mode != null || !isSettled;
    const disabledRedo = !gameMgr || mode != null || !history || !history.redoable;
    const disabledStep = !gameMgr || isGameOver || mode !== null || (isExternal === enableAI);
    const disableSemi = !gameMgr || isGameOver || (mode !== null && mode !== 'semi') || !hasBest;
    const disableSemiE = !gameMgr || isGameOver || mode !== null || !hasBest;
    const disableAuto = !gameMgr || isGameOver || (mode !== null && mode !== 'auto');
    const disableAutoE = !gameMgr || isGameOver || mode !== null;
    const textUndo = isExternal && !enableAI ? 'Clear' : 'Undo';
    const intentRestart = isManual ? 'warning' : isGameOver ? isWon ? 'success' : 'danger' : undefined;
    const textRestart = isManual ? 'Reload' : 'New game';
    const textStep = isExternal ? 'Solve' : 'Single step';
    const tooltipDrain = drainable
        ? 'Analyze all possible solutions and find the optimal click sequence'
        : 'Only available when there are fewer than 256 possible solutions';

    if (dim.width <= width * 19.3 + 446)
        return (
            <Card elevation={Elevation.TWO} className="control compact">
                <ControlGroup>
                    <ButtonGroup>
                        <Tooltip content="Close" position="top">
                            <Button disabled={disabledGeneral} icon="arrow-left" onClick={onStop} />
                        </Tooltip>
                        <Tooltip content={textRestart} position="top" intent={intentRestart}>
                            <Button disabled={disabledGeneral} icon="refresh" intent={intentRestart} onClick={onRestart} />
                        </Tooltip>
                    </ButtonGroup>
                    <ButtonGroup>
                        <Tooltip content={textUndo} position="top">
                            <Button disabled={disabledUndo} icon="undo" onClick={onUndo} />
                        </Tooltip>
                        <Tooltip content="Save game to file" position="top">
                            <Button disabled={disabledSave} icon="download" onClick={onDownload} />
                        </Tooltip>
                        <Tooltip content="Redo" position="top">
                            <Button disabled={disabledRedo} icon="redo" onClick={onRedo} />
                        </Tooltip>
                    </ButtonGroup>
                    {!isExternal && (
                        <ButtonGroup>
                            <Tooltip content="Auto-restart" position="top">
                                <Button active={isAutoRestart} icon="repeat" onClick={onSwitchRestart}
                                    intent={isAutoRestart ? "success" : undefined} />
                            </Tooltip>
                            <Tooltip content="Enable AI" position="top">
                                <Button active={enableAI} icon="intelligence" onClick={onSwitchAI}
                                    intent={enableAI ? "primary" : undefined} />
                            </Tooltip>
                            <Tooltip content="Auto-flag" position="top">
                                <Button active={isAutoFlag} icon="flag" onClick={onSwitchFlag}
                                    intent={isAutoFlag ? "danger" : undefined} disabled={!enableAI} />
                            </Tooltip>
                        </ButtonGroup>
                    )}
                </ControlGroup>
                {(enableAI && !isDraining) && (
                    <ControlGroup>
                        <Tooltip content={textStep} position="bottom">
                            <Button disabled={disabledStep} icon="hand-up" intent="primary" onClick={onStep}
                                    onMouseEnter={() => setIsStepHover(true)} onMouseLeave={() => setIsStepHover(false)} />
                        </Tooltip>
                        <ButtonGroup className="short-btns">
                            <Tooltip content="Increase speed" placement="bottom">
                                <Button icon="plus" onClick={onSpeedUp} />
                            </Tooltip>
                            <Tooltip content="Decrese speed" placement="bottom">
                                <Button icon="minus" onClick={onSpeedDown} />
                            </Tooltip>
                        </ButtonGroup>
                        <ButtonGroup>
                            <Tooltip content="Open 100% safe blocks on by one" placement="bottom">
                                <Button disabled={disableSemi} active={mode === 'semi'}
                                        icon="play" intent="success" className="growing" onClick={onSemi} />
                            </Tooltip>
                            <Tooltip content="Open all safe blocks" placement="bottom">
                                <Button disabled={disableSemiE} icon="fast-forward" intent="success" onClick={onSemiEvery} />
                            </Tooltip>
                            <Tooltip content="Open all safe blocks, recursively" placement="bottom">
                                <Button disabled={disableSemiE} icon="step-forward" intent="success" onClick={onSemiAll} />
                            </Tooltip>
                        </ButtonGroup>
                        <ButtonGroup>
                            <Tooltip content="Let AI play the game" placement="bottom">
                                <Button disabled={disableAuto} active={mode === 'auto'}
                                        icon="rocket-slant" intent="warning" className="growing" onClick={onAuto} />
                            </Tooltip>
                            <Tooltip content="Let AI finish the game" placement="bottom">
                                <Button disabled={disableAutoE} icon="lightning" intent="warning" onClick={onAutoAll} />
                            </Tooltip>
                        </ButtonGroup>
                        <ButtonGroup>
                            <Tooltip content={tooltipDrain} placement="bottom-end">
                                <Button active={isDrain} icon="layout-balloon" intent={drainable ? 'danger' : undefined}
                                        disabled={!drainable || mode !== null} onClick={onDrainAlert} />
                            </Tooltip>
                        </ButtonGroup>
                    </ControlGroup>
                    )}
                {isDraining && (
                    <FormGroup label={isDraining[0] === isDraining[1]
                            ? 'Post-processing results...'
                            : `${isDraining[0]}/${isDraining[1]} solutions analyzed`}>
                    <ProgressBar value={isDraining[0] / isDraining[1]} intent="danger"
                                 stripes={isDraining[0] === isDraining[1]} />
                    </FormGroup>
                )}
            </Card>
        );

    const short = dim.height < 600;

    const speedSlider = (
        <Tooltip content="Speed" placement="left-end">
            <FormGroup>
                <Slider fill min={-2} max={2} stepSize={0.1} labelStepSize={1} onChange={setSpeed}
                    labelRenderer={renderLabel} showTrackFill={false} value={speed} />
            </FormGroup>
        </Tooltip>);
    const stepButton = (
        <Button disabled={disabledStep} text={textStep} onClick={onStep}
                icon="hand-up" intent="primary" className="growing" fill
                onMouseEnter={() => setIsStepHover(true)} onMouseLeave={() => setIsStepHover(false)} />);

    return (
        <Card elevation={Elevation.TWO} className="control" compact={short}>
            {!short && (<h3>Game Control{isManual ? ' (loaded)' : ''}</h3>)}
            <ControlGroup vertical>
                <ButtonGroup>
                    <Button disabled={disabledGeneral} icon="cross"
                            className="growing" text="Close" onClick={onStop} />
                    <Button disabled={disabledGeneral} rightIcon="refresh" intent={intentRestart}
                            className="growing" text={textRestart} onClick={onRestart} />
                </ButtonGroup>
                <ButtonGroup>
                    <Button disabled={disabledUndo} icon="undo"
                            className="growing" text={textUndo} onClick={onUndo} />
                    <Tooltip content="Save game to file" position="bottom">
                        <Button disabled={disabledSave} rightIcon="download"
                                className="growing" onClick={onDownload} />
                    </Tooltip>
                    <Button disabled={disabledRedo} rightIcon="redo"
                            className="growing" text="Redo" onClick={onRedo} />
                </ButtonGroup>
            </ControlGroup>
            {!isExternal && (<>
                <br />
                <Switch checked={isAutoRestart} onChange={onSwitchRestart}
                        labelElement={'Auto-restart'}
                        innerLabelChecked="on" innerLabel="off"
                        alignIndicator={Alignment.RIGHT} />
                <Switch checked={enableAI} onChange={onSwitchAI}
                        labelElement={'Enable AI'} disabled={mode !== null}
                        innerLabelChecked="on" innerLabel="off"
                        alignIndicator={Alignment.RIGHT} />
                </>)}
            <Collapse isOpen={enableAI || isExternal} keepChildrenMounted>
                {!short && (<h3>AI Control</h3>)}
                {!isExternal && (
                    <Switch checked={isAutoFlag} onChange={onSwitchFlag}
                            labelElement={'Auto-flag'}
                            innerLabelChecked="on" innerLabel="off"
                            alignIndicator={Alignment.RIGHT} />
                    )}
                {!short && stepButton}
                {!isExternal && (<>
                    {!short && (<br />)}
                    {!short && speedSlider}
                    <ControlGroup vertical>
                        {short && stepButton}
                        <ButtonGroup>
                            <Button disabled={disableSemi}
                                    active={mode === 'semi'}
                                    icon="play" intent="success" className="growing"
                                    text="Semi-auto" onClick={onSemi} />
                            <Button disabled={disableSemiE}
                                    icon="fast-forward" intent="success"
                                    onClick={onSemiEvery} />
                            <Button disabled={disableSemiE}
                                    icon="step-forward" intent="success"
                                    onClick={onSemiAll} />
                        </ButtonGroup>
                        <ButtonGroup>
                            <Button disabled={disableAuto}
                                    active={mode === 'auto'}
                                    icon="rocket-slant" intent="warning" className="growing"
                                    text="Full-auto" onClick={onAuto} />
                            <Button disabled={disableAutoE}
                                    icon="lightning" intent="warning"
                                    onClick={onAutoAll} />
                        </ButtonGroup>
                        {!isDraining && (
                            <Tooltip content={tooltipDrain} placement="bottom-end">
                                <ButtonGroup>
                                    <Button active={isDrain} icon="layout-balloon" rightIcon="layout-balloon"
                                            intent={drainable ? 'danger' : undefined} className="growing" disabled={!drainable || mode !== null}
                                            text="Exhaustive Search" onClick={onDrainAlert} />
                                </ButtonGroup>
                            </Tooltip>
                            )}
                        {isDraining && (
                            <FormGroup label={isDraining[0] === isDraining[1]
                                    ? 'Post-processing results...'
                                    : `${isDraining[0]}/${isDraining[1]} solutions analyzed`}>
                            <ProgressBar value={isDraining[0] / isDraining[1]} intent="danger"
                                         stripes={isDraining[0] === isDraining[1]} />
                            </FormGroup>
                        )}
                    </ControlGroup>
                    {short && speedSlider}
                    </>)}
            </Collapse>
            {isExternal && (<>
                <h4>External Mode</h4>
                <p>Use left mouse click to indicate the degree of each block. Use right click to toggle between 'e' (assumed not to have mine) and 'M' (assumed to have mine.) Once you click Solve, you <strong>cannot</strong> edit existing degrees.</p>
                </>)}
        </Card>
    );
}
