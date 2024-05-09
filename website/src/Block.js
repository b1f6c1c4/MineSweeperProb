import React, { useRef } from 'react';
import { Icon } from '@blueprintjs/core';

export default function Block(props) {
    const {
        isGameOver,
        isWon,
        isOpen,
        degree,
        isHover,
        isFlagged,
        isEFlagged,
        hasMine,
        probability,
        isBest,
        isPreferred,
        isSafe,
        isDangerous,
        isDrain,
        isLastProbe,
        row,
        col,
        onProbe,
        onFlag,
        onHover,
        onUnHover,
    } = props;

    let tdClass = 'blk';
    let tdStyle = {};
    let spanClass = '';
    let text = '';

    if (isOpen) {
        tdClass += ' blk-open';
        if (hasMine) {
            tdClass += ' blk-dead';
            text = 'M';
        } else if (isEFlagged) {
            spanClass = 'flag';
            text = 'e';
        } else if (degree) {
            spanClass = `num-${degree}`;
            text = degree;
        }
    } else if (!isGameOver && !isWon) {
        if (isFlagged) {
            spanClass = 'flag';
            text = 'F';
        }
        if (isSafe)
            tdClass += ' blk-safe';
        else if (isDangerous)
            tdClass += ' blk-dangerous';
        else if (probability !== null) {
            if (!isDrain) {
                const r = Math.round(0xf8 * probability + 0x09 * (1 - probability));
                const g = Math.round(0x29 * probability + 0xb7 * (1 - probability));
                const b = Math.round(0x29 * probability + 0x22 * (1 - probability));
                tdStyle.backgroundImage = `linear-gradient(135deg, rgb(${r}, ${g}, ${b}), #d2d2d2 40%)`;
            } else {
                const r = Math.round(0x7b * probability + 0xf8 * (1 - probability));
                const g = Math.round(0x68 * probability + 0x29 * (1 - probability));
                const b = Math.round(0xee * probability + 0x29 * (1 - probability));
                tdStyle.backgroundImage = `linear-gradient(45deg, rgb(${r}, ${g}, ${b}), transparent 100%)`;
            }
        }
        if (isBest)
            tdClass += ' blk-best';
        if (isPreferred)
            tdClass += ' blk-preferred';
        if (isDrain)
            tdClass += ' blk-drain';
    } else {
        if (isFlagged && !hasMine) {
            spanClass = 'flag bad-flag';
            text = 'F';
        } else if (isFlagged) {
            spanClass = 'flag';
            text = 'F';
        } else if (!isWon && hasMine) {
            text = 'M';
        }
        if (isWon)
            tdClass += ' blk-won';
    }

    const hasLP = useRef(false);
    const lastLPTU = useRef(0);
    const canceller = useRef(undefined);
    function onClick() {
        if (new Date() - lastLPTU.current < 500) {
            return;
        }
        onProbe(row, col);
    }
    function onRClick(e) {
        onFlag(row, col);
        e.preventDefault();
    }
    function onML() {
        cancel();
        onUnHover();
    }
    function onTS() {
        hasLP.current = false;
        canceller.current = setTimeout(() => {
            onFlag(row, col);
            hasLP.current = true;
        }, 500);
    }
    function cancel() {
        if (canceller.current) {
            clearTimeout(canceller.current);
            canceller.current = undefined;
        }
        if (hasLP.current)
            lastLPTU.current = new Date();
    }

    return (<td className={tdClass}
                style={tdStyle}
                onMouseEnter={onHover}
                onMouseLeave={onML}
                onTouchStart={onTS}
                onTouchEnd={cancel}
                onClick={onClick}
                onContextMenu={onRClick}>
        {(isLastProbe || (isHover && !isOpen)) && (
            <Icon icon="locate" size={20} />
        )}
        <span className={spanClass}>{text}</span>
    </td>);
}
