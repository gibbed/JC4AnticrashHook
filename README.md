# Just Cause 4 Anticrash Hook

This is an anticrash hook that I developed to counter a fairly consistent crash that I (gibbed) am getting in Just Cause 4.

I'm releasing it because a few people have expressed interest in it.

Like what I've done? **Consider supporting me on [Patreon](https://patreon.com/gibbed) or [Ko-fi](https://ko-fi.com/gibbed)**.

## Caveats

* This is a workaround for a D3D11 crash that Just Cause 4 is causing. **If you're playing in D3D12 (Windows 10), this will very likely not help you.**
* This is only a workaround for a **SINGLE** very specific crash. **If you have another issue, well, this won't help you.**
* This hook applies to the first version of Just Cause 4 released on Steam. **No other version.**
* There is a performance hit for using this hook. How significant? I couldn't tell you.
* There is a possibility of incorrect graphics when the issue occurs. Can't do anything about that.
* If you have any software that does anything funny to D3D11, it's possible this hook will not work properly. Can't do anything about that.

## Usage

* [Download the latest binary release](https://github.com/gibbed/JC4AnticrashHook/releases/latest) (not the source ZIP!).
* Extract `XInput9_1_0.dll` to the game directory (where `JustCause4.exe` is).
* Play.

If things are happening, a console window will open once Just Cause 4 starts.
