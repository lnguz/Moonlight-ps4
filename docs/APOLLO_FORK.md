# Apollo fork (1.4.2)

Changes on top of Akryst's Moonlight PS4 1.4.1. Same title ID (`MLNT00001`),
so the package installs as an update and keeps `/data/moonlight` (pairing,
PCs and settings).

## Apollo
- **Virtual display**: every launch sends `virtualDisplay` and `scaleFactor`
  (Settings -> Apollo). Apollo creates a monitor at the stream's resolution and
  FPS. Plain Sunshine ignores both parameters.
- **Device name**: pairing reports `device_name` (default `PS4`) instead of the
  hardcoded `roth`. Existing pairings keep their old name; rename them in
  Apollo's web UI.
- **PC commands**: Apollo's server commands appear in the stream menu
  (hold L3 + R3 -> PC commands). Requires the "server commands" permission for
  this client in Apollo. Protocol: control message `0x3000`
  (`patches/moonlight-common-c-apollo.patch`).

## Controller
- **Touchpad click** (Settings -> Controller): `Select` (default, sends
  Back/Select), `Mouse click` (old behaviour) or `DS4 touchpad`. With touchpad
  mouse on, the finger still moves the pointer in every mode.
- Home/Guide stays on **L1 + R1 + Options**.
- Controller state is sent on change (plus a 250 ms refresh) instead of every
  4 ms poll.

## Network and latency
- **Wi-Fi keep-alive** (Settings -> Connection, default on): a tiny unreliable
  control ping every 20 ms keeps the console's Wi-Fi out of power save.
- **Frame pacing** (Settings -> Video): `Lowest latency` decodes every frame but
  only shows the newest one when a newer frame is already queued.
- `present_late_us` INI key (default 33000, 0 = never skip): frames older than
  this are decoded but not shown.

## Other
- **Wake PC** (Settings -> Connection): sends Wake-on-LAN on demand. The PC's
  MAC is learned automatically when you connect while it is on.

New INI keys: `device_name`, `virtual_display`, `vd_scale`, `touchpad_click`,
`wifi_keepalive`, `latest_frame`, `present_late_us`.

## 1.4.3
- **No more slideshow at high bitrate**: a late frame is only hidden when a
  newer frame is already queued. Before, when the hardware decoder ran close
  to its 16.7 ms budget (busy scenes at 25-50 Mbps), every frame counted as
  late and the picture dropped to 1-2 fps.
- **Home hold**: L1 + R1 + Options now holds Guide for as long as the chord is
  held (long press menus), and at least 80 ms on a tap.
- **PC IP change**: the PC's `<uniqueid>`/hostname is saved after each connect
  (`host_uuid`, `host_name`). When the saved address stops answering, the 128
  nearest addresses are probed in parallel and the same PC is followed to its
  new address automatically.
