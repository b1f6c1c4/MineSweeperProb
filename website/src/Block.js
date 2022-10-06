import React from 'react';

export default function Block(props) {
    const {
        isGameOver,
        isWon,
        isOpen,
        degree,
        isFlagged,
        hasMine,
        probability,
        isBest,
        isPreferred,
        isSafe,
        isDangerous,
        row,
        col,
        onProbe,
        onFlag,
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
            const r = Math.round(0xf8 * probability + 0x09 * (1 - probability));
            const g = Math.round(0x29 * probability + 0xb7 * (1 - probability));
            const b = Math.round(0x29 * probability + 0x22 * (1 - probability));
            tdStyle.backgroundImage = `linear-gradient(135deg, rgb(${r}, ${g}, ${b}), #d2d2d2 40%)`;
        }
        if (isBest)
            tdClass += ' blk-best';
        if (isPreferred)
            tdClass += ' blk-preferred';
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

    function onClick(e) {
        if (e.type === 'click')
            onProbe(row, col);
        else if (e.type === 'contextmenu') {
            onFlag(row, col);
            e.preventDefault();
        }
    }

    return (<td className={tdClass}
                style={tdStyle}
                onClick={onClick}
                onContextMenu={onClick}>
        <span className={spanClass}>{text}</span>
    </td>);
}
