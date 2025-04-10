# Flammyx Phase 2 Optimization - Latency Reduction

**Objective:** Reduce gesture recognition processing latency from ~30ms to <16ms to meet real-time requirements for Flammyx effects and combat integration.

**Date:** April 11, 2024

## Profiling Results (CSLTest.exe - April 11)

Profiling was conducted using `CSLTest.exe` after ensuring 100% accuracy on `GestureTest.exe`.

**Key Findings:**

*   **Overall Latency:** The primary measurement, `processSimulatedPoints` execution time, consistently falls between **27ms and 31ms** per gesture recognition cycle. This exceeds the <16ms target.
*   **Individual Function Durations:**
    *   `calculateSwipeConfidence`: Very fast (< 0.1ms). Not a bottleneck.
    *   `recognizeStasai` (including `isCircle`): Moderate latency (~1-2ms). Potential for minor optimization.
    *   `recognizeKhargail`/`Flammil`/`Annihlat`: Low individual latency, primarily driven by `calculateSwipeConfidence`.
*   **Velocity Processing:** The block responsible for calculating, smoothing (EMA), and normalizing velocity vectors within `processSimulatedPoints` involves multiple loops and calculations (sqrt) and likely contributes significantly to the overall latency.

**Conclusion:** The primary bottleneck is not a single slow function but rather the cumulative cost of:
1.  Sequentially checking *all* gesture types (`recognize*`) on every input frame.
2.  The velocity processing steps performed after identifying the best-matching gesture candidate.

## Refined Optimization Plan

Based on the profiling results, the following optimization strategies will be pursued:

1.  **Optimize `processSimulatedPoints` Recognition Loop:**
    *   **Strategy:** Implement an "early exit" mechanism. If a gesture (e.g., Khargail) is detected with confidence significantly above its threshold (e.g., > 0.95), potentially skip the checks for other gestures in that frame.
    *   **Trade-offs:** Need to balance latency savings against the small risk of missing a rapidly transitioning secondary gesture. Thresholds for early exit must be chosen carefully.
2.  **Optimize Velocity Calculation Block:**
    *   **Strategy:** Review the velocity calculation, EMA smoothing, and normalization logic.
        *   Explore possibilities for vectorization or parallelization if applicable (consider complexity vs. benefit).
        *   Reduce loop overhead or redundant calculations.
        *   Investigate replacing `sqrt` with faster approximations if precision allows.
    *   **Trade-offs:** Ensure optimization doesn't negatively impact the quality or usefulness of the velocity data for downstream systems (like Flammyx effects).
3.  **Targeted `isCircle` Optimization:**
    *   **Strategy:** Modify `isCircle` to sample fewer points when calculating the average radius and standard deviation (e.g., every 2nd or 3rd point) instead of iterating over all points in the trajectory.
    *   **Trade-offs:** Minor potential impact on circle detection accuracy, requires testing. Expected latency saving is likely small but contributes to the overall goal.

## Next Steps

1.  **Implement Optimizations:** Prioritize strategies 1 and 2.
2.  **Profile Again:** Re-run `CSLTest.exe` after each significant optimization to measure impact.
3.  **Sync with Marcus (April 11, 10:30 AM PST):** Present findings, discuss plan and trade-offs, align on latency targets for hitbox tuning.
4.  **Iterate:** Continue optimization until <16ms target is achieved. 