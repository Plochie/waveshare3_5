import PairPanel, { PendingPair } from "./PairPanel";

interface Props {
  pending: PendingPair | null;
  onResolved: () => void;
  onSkip: () => void;
}

export default function Onboarding({ pending, onResolved, onSkip }: Props) {
  return (
    <div className="onboarding">
      <div className="ob-left">
        <div className="ob-glyph" />
        <h1 className="ob-title">Connect your deck</h1>
        <p className="ob-subtitle">Let's get your Stream Deck paired.</p>
        <ol className="ob-steps">
          <li>
            <span className="ob-num">1</span>Power on your deck
          </li>
          <li>
            <span className="ob-num">2</span>Join the same Wi-Fi network
          </li>
        </ol>
        <button type="button" className="ob-skip" onClick={onSkip}>
          Skip for now
        </button>
      </div>
      <div className="ob-right">
        <PairPanel pending={pending} context="onboarding" onResolved={onResolved} />
      </div>
    </div>
  );
}
