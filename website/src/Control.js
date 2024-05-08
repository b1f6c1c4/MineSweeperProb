import React from 'react';
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
        isReady,
        isSettled,
        isWon,
        loadedGame,
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
        onSwitch,
        onSwitchAI,
        onSwitchFlag,
        onUndo,
        rate,
        renderLabel,
        setIsStepHover,
        setSpeed,
        speed,
    } = props;

    const drainable = isReady && !isGameOver && rate[0] <= 8 && !isDrain;

    return (
        <Card elevation={Elevation.TWO} className="control">
            <h3>Game Control{loadedGame ? ' (loaded)' : ''}</h3>
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
                    <Tooltip content='Save game to file' position='bottom'>
                        <Button disabled={!gameMgr || mode != null || !isSettled} rightIcon="download"
                                className="growing" onClick={onDownload} />
                    </Tooltip>
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
                                text={isExternal ? 'Solve' : 'Single step'} onClick={onStep}
                                onMouseEnter={() => setIsStepHover(true)} onMouseLeave={() => setIsStepHover(false)} />
                    </ButtonGroup>
                </ControlGroup>
                {!isExternal && (<>
                    <br />
                    <Tooltip content="Speed" placement="left-end">
                        <FormGroup>
                            <Slider
                                fill
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
                    </Tooltip>
                    <ControlGroup vertical>
                        <ButtonGroup>
                            <Button disabled={!gameMgr || isGameOver || (mode !== null && mode !== 'semi') || !hasBest}
                                    active={mode === 'semi'}
                                    icon="play" intent="success" className="growing"
                                    text="Semi-auto" onClick={onSemi} />
                            <Button disabled={!gameMgr || isGameOver || mode !== null || !hasBest}
                                    icon="fast-forward" intent="success"
                                    onClick={onSemiEvery} />
                            <Button disabled={!gameMgr || isGameOver || mode !== null || !hasBest}
                                    icon="step-forward" intent="success"
                                    onClick={onSemiAll} />
                        </ButtonGroup>
                        <ButtonGroup>
                            <Button disabled={!gameMgr || isGameOver || (mode !== null && mode !== 'auto')}
                                    active={mode === 'auto'}
                                    icon="rocket-slant" intent="warning" className="growing"
                                    text="Full-auto" onClick={onAuto} />
                            <Button disabled={!gameMgr || isGameOver || mode !== null}
                                    icon="lightning" intent="warning"
                                    onClick={onAutoAll} />
                        </ButtonGroup>
                        {!isDraining && (
                            <Tooltip
                                content={drainable
                                        ? 'Analyze all possible solutions and find the optimal click sequence'
                                        : 'Only available when there are fewer than 256 possible solutions'}
                                placement="bottom-end"
                            >
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
                    </>)}
            </Collapse>
            {isExternal && (<>
                <h4>External Mode</h4>
                <p>Use left mouse click to indicate the degree of each block. Use right click to toggle between 'e' (assumed not to have mine) and 'M' (assumed to have mine.) Once you click Solve, you <strong>cannot</strong> edit existing degrees.</p>
                </>)}
        </Card>
    );
}
