# Rolling stock: the train itself

The game's train fills one whole long side of the platform as a wall, and later
needs to be modelled, dressed and made to feel lived in. This file records the
physical form. No operator livery, colour stripe arrangement or logo is
described or reproduced; the shape, structure and wear are fair game.

## The decision: the game's train is the Aventra-derived one

The fictional line is a fictionalised Crossrail-scale line: a main-line
loading-gauge line modelled on the Elizabeth line, NOT a deep-level tube. The
game's train is therefore modelled on the **Class 345 "Aventra"** silhouette
and the platform is a **main-line loading-gauge box** (large cross-section,
near-vertical walls, a tall tiled platform tunnel or sub-surface station hall).
This is the canonical decision; the Aventra section below is the primary spec.
The Jubilee-line 1996 Stock is kept only as a short "what we are NOT building"
contrast at the end of this file.

## Class 345 "Aventra" (main line gauge) - the train to build

Built to the British main line loading gauge, walk-through, more like a
suburban electric multiple unit than a tube train.

- **Car length**: 23.615 m driving motor cars, 22.500 m intermediate cars.
- **Body width**: 2.772 m (about 9 ft 1 in). Near vertical sides, only a slight
  tumblehome, a box not a tube.
- **Body height**: 3.760 m rail to roof.
- **Floor height above rail**: 1.145 m.
- **Train length**: 204.73 m as a 9-car unit. 9 cars per unit.
- **Doors**: 3 per side per car, double-leaf sliding plug doors, each leaf pair
  1.450 m wide.
- **Walk-through**: YES. Full open gangways between every car, you can see the
  whole train end to end down the centre. Wide bellows connection between cars.
  This matters for the "train as a wall" view: it reads as one continuous
  interior lit strip, NOT distinct car bodies with black articulation gaps.
- **Capacity**: 1500 per 9-car unit (454 seated, 1046 standing).
- **Seating**: mix of longitudinal (bench along the wall, near the doors) and
  transverse (bays facing each other, further in).
- **Cab front**: a full-width curved wraparound windscreen (the "smiling"
  front), not a raked flat pane. Headlight and marker clusters low on the nose,
  a coupler cover below a nose cone.
- **Roof and skirt**: flat roof with air-conditioning pods, long and low
  overall, a deep skirt (side valance) below the solebar hiding the underframe
  equipment run.
- **Lighting**: LED strip lighting the length of the ceiling each side, flat,
  bright and even.
- **Passenger information**: ceiling-mounted / cant-rail LCD strip displays
  running along above the windows, plus screens at the car ends. Describe
  placement, not content.
- Platform screen doors in the central (Canary Wharf) tunnel section; a
  fictionalised main-line station may or may not have them (`bHasPlatformScreenDoors`
  is a per-station flag in the design docs).
- Interior is bright, high, airy, "new suburban train", NOT "tube".

## Main line loading-gauge body profile (the shape to model)

Model this precisely, it is the silhouette that sells "modern London rail":

- The cross section is close to a **rounded rectangle**: near-vertical sides
  with only a slight tumblehome, a flat roof, generous corner radii at the cant
  rail and the solebar. A box, not a tube worm.
- **The sides are tall and near-vertical.** The widest point is only slightly
  proud of the roof and floor lines; there is no hard inward curve at knee
  height. The body reads as a wall from the platform.
- **The doors are flat plug doors**: they push out slightly and slide along the
  outside of the body, flush when closed. The doorway is a plain tall
  rectangle, not narrowed top and bottom.
- **Seats sit against a near-vertical wall** so longitudinal benches run
  straight; there are no splayed diagonal end seats forced by a taper.
- The window band is a tall, near-continuous glazed strip, large windows,
  softly radiused corners.
- Overall read from the platform: a long, low, flat-roofed box, one continuous
  lit interior visible through the glass end to end, a deep dark skirt beneath,
  the curved cab front at the leading end.

## Doors

- **Double-leaf sliding plug doors** (Aventra), 3 per side per car. Each leaf
  pushes out a few centimetres then slides along the outside of the body,
  sitting flush with the body skin when closed. Each leaf pair is about 1.45 m
  wide.
- **Rubber edge seals**: a soft rubber nose down the leading edge of each leaf,
  they meet in the centre with a compressible bulb seal. Perished, grey,
  scuffed, sometimes torn on an old train.
- **Door control buttons**: the Aventra HAS passenger open buttons, lit, on both
  sides of every doorway, inside and outside, at about 1.0 to 1.1 m. An
  emergency door release (a covered handle or flap behind a "break glass" style
  cover, at about 1.3 m beside each doorway) lets a passenger open a door
  manually.
- **The gap and step**: between platform edge and train there is a modest
  horizontal gap and usually a small vertical step, train floor slightly above
  or below platform. The Aventra floor is 1.145 m above rail; on a straight
  main-line platform the gap is small.
- **Platform edge treatment**: a painted or inlaid warning strip set back from
  the edge, the tactile blister band behind that (66.5 mm dome spacing, see
  materials file), and repeated floor text warning of the gap. Recreate as an
  original mark; do not reproduce the exact "mind the gap" artwork or the
  recorded announcement.
- **Door threshold**: a metal sill plate, ridged or chequered, the single most
  worn spot on the whole train, scuffed to bright bare metal, black with ground
  in dirt at the edges, sometimes a slightly buckled or lifted corner.

## Interior

- **Layout (Aventra)**: mixed **longitudinal bench seating** (side-facing, along
  the wall near the doors) and **transverse bays** (facing each other, further
  in). Wide standing area by each doorway. Benches run straight against a
  near-vertical wall, no splayed diagonal end seats.
- **Moquette**: seats are upholstered in a **hard-wearing patterned wool
  moquette** (a dense cut-pile velvet-like woven fabric). The pattern exists
  for a practical reason: a busy multi-colour weave hides dirt, wear and stains,
  and the pile is tough enough for millions of sits. Describe and design an
  ORIGINAL pattern in the game palette; do NOT reproduce any operator's actual
  moquette design. The concept to model: a repeating geometric or abstract
  multi-tone weave, matte, slightly fuzzy, flattened and shiny where people sit,
  gum and chewing marks underneath the seat lip.
- **Priority seats**: a small number of seats near each doorway are marked as
  priority (for disabled, elderly, pregnant passengers) usually by a different
  moquette colour and a small pictogram sign above. Describe as an original mark.
- **Grab poles**: vertical stainless poles floor to ceiling by every doorway and
  at intervals down the car, sometimes a **branching pole** (one pole splitting
  into three near the ceiling) in the door vestibule. Horizontal grab rails run
  along under the luggage line at about 1.8 m.
- **Ceiling grab handles**: sprung "strap hangers", a plastic triangular or
  ball handle on a short flexible stalk or a rubber loop, hanging from the
  horizontal rail, swinging. Rows of them down the standing area.
- **Glazed draught screens**: a toughened glass panel at the end of each seat
  run next to the doorway, in a stainless frame with a grab handle cut or
  moulded into the vertical edge, to shelter seated passengers from the door
  draught. Greasy handprints on the glass at grip height.
- **Wheelchair / multi-use bay**: a clear floor area with tip-up seats and a
  backrest pad on the wall, a lower horizontal grab rail, near a doorway.
- **Passenger information displays (Aventra)**: **LCD strip displays** running
  along above the windows on the cant rail, plus screens at the car ends.
  Describe placement (along the cant rail, car ends) not content.
- **Interior lighting (Aventra)**: **LED strip** the length of the ceiling each
  side, flat, bright and even. For a survival-horror dressing: some strips
  dimmed, flickering or dead, a car in darkness, emergency lighting only (a
  dimmer warm or blue-white strip).
- **Ventilation grilles**: slotted or perforated metal grilles in the ceiling
  and at the car ends, greasy, dust-furred.
- **Emergency equipment**: an emergency door release by each doorway (see
  Doors), a passenger emergency alarm handle (yellow or red, roughly 1.5 m, one
  per doorway area), a small fire extinguisher in the cab and sometimes a
  saloon location.
- **CCTV**: small black or white **dome cameras** in the ceiling, two to four
  per car, near the doorways.
- **Floor**: a hard-wearing sheet floor covering, often with a raised stud or
  ribbed texture in the door vestibule for grip, worn smooth in the walking
  paths, black scuffed at the thresholds, gum-spotted.
- **Windows**: large side windows, some with a small hinged hopper vent at the
  top. Etched and scratched (idle vandalism), hazy, greasy at head-lean height.

## Cab end

- **Aventra cab**: a full-width driving cab at each end of the unit with a
  **curved wraparound windscreen** (the "smiling" front, not a raked flat
  pane), wipers, headlight and marker light clusters low on the nose, a nose
  cone over the coupler.
- **Emergency end door**: main-line units carry a **detrainment door or
  fold-down ramp behind a panel in the cab front**, so passengers can be
  detrained forward onto the track if the train is stuck between stations. This
  is directly relevant to gameplay: it is a plausible escape route or ingress
  point at the cab end of the train wall. When deployed it is a doorway at floor
  level in the front of the unit, with a short ramp or steps down to track
  level.
- **Coupling**: an automatic (Wedgelock or similar) coupler behind a nose cone
  or a hinged cover, with electrical and air connections. Only visible if the
  nose cover is open or the unit is uncoupled.
- **Grab handles and footholds by the cab door**: a vertical grab rail and one
  or two recessed step rungs beside the cab door on the body side, for crew
  access from track level.

## Exterior detail for the "train as a wall" view

This is the face the player sees for most of the round. Model the band from
roughly knee height to head height in detail, less above and below.

- **Body panel lines**: horizontal shadow lines where body panels meet (waist
  rail, below the windows, at the solebar), vertical lines at the door edges and
  car ends. Aluminium bodyside, subtle orange-peel in the paint, slight ripple
  between the internal frames.
- **Between-car connection (Aventra)**: a wide full-height corrugated bellows
  you can walk through. From the platform the gap between cars reads as a soft
  concertina, not a black void, and the lit interior is continuous through it.
  This is the key "not a segmented tube train" tell in the train-as-a-wall view.
- **Underframe equipment boxes**: below the solebar, a run of grey or black
  boxes (traction equipment, batteries, control gear, air reservoirs, resistor
  grilles) slung between the bogies, on a rectangular subframe. Grilled faces,
  cable runs, pipe runs, hazard labels, earth straps.
- **Bogies (wheel trucks)**: one at each end of every car, a fabricated steel
  frame carrying two axles, coil and rubber springs, dampers, brake discs or
  tread brake blocks, the traction motor slung on the motor bogies. Caked in
  brown-grey brake dust and oily grime. This is the dirtiest part of the train.
- **Shoegear / collector shoes**: through the third-rail core section the bogies
  carry a shoe beam with a pickup shoe riding on the conductor rail, a bright
  arcing-scorched contact face and a heavy flexible cable to the equipment.
  Outside the core the unit collects from the overhead line via the roof
  pantograph instead.
- **Roof detail (Aventra)**: a flat roof with roof-mounted air-conditioning
  pods, aerials, a pantograph well (it runs on 25 kV overhead outside the core
  and on third rail through it), a walkway strip; mostly matte grimy grey,
  rain-streaked.
- **Route / destination display box**: a small illuminated dot-matrix or roller
  box on the cab front and often a repeater on the bodyside at the leading end
  of each car, showing the line and destination. Describe placement (cab front
  centre or upper corner, bodyside at car ends), not content.
- **Numbering and markings**: car running numbers near the cab and at the
  bodyside ends, small stencilled data panels (weights, lifting points,
  electrical warnings), a "do not ride on this" flash by the shoegear. Keep any
  in-world numbering original.

## Wear and life (what makes it lived in)

| Location | Wear |
|---|---|
| Door threshold sill plate | worn to bright bare metal in the centre, black grime at the edges, occasional buckle |
| Door rubber seals | perished grey, scuffed, small tears, chewing-gum on the lower nose |
| Floor at doorways and down the aisle | polished-smooth walking paths, black scuff at thresholds, gum spots |
| Grab poles and rails at hand height (0.9 to 1.7 m) | polished bright and slightly greasy, dull above and below the grip zone |
| Strap hangers | grimy handle, cracked plastic, some missing leaving a bare loop |
| Draught screen glass and window at lean height | greasy handprints and forehead smears, hazing, fine scratch-etching |
| Moquette seats | flattened and sheened in the sitting hollows, frayed at the front edge, gum and marks under the lip, a stain or two |
| Seat end panels and screen frames | scratched, stickered, tag marks, kicked lower corners |
| Ceiling above lights and vents | yellowed diffusers, dust-furred grilles, dead or flickering tubes |
| Bodyside below the windows | tunnel dirt: horizontal grey-brown streaks trailing back from every rivet, joint and door edge, heavier at the lower body |
| Bodyside above the windows and roof | sooty grey film, rain-streaked, aerial bases rust-bled |
| Underframe and bogies | thick brown-grey brake dust, oily black around dampers and gearboxes, salt-white crust in winter |
| Shoegear contact face | bright metallic, blue-scorched, sparks in the dark |
| Between-car gap | oily, cabled, littered, a dropped freesheet wedged in the coupler |
| Cab front | grimy, fly-spattered lower half, wiper-swept arc clean on the screen, chipped paint on the nose |

## Open 3D references

| Name | URL | Licence | Format | Note |
|---|---|---|---|---|
| Bombardier S Stock London Underground (timblewee) | https://sketchfab.com/3d-models/bombardier-s-stock-london-underground-a6718eff2dc843c48fd54376b4c70b06 | CC-BY 4.0 | glTF, rigged sliding doors | Sub-surface stock: full-width near-vertical box body, the closest free model to the Aventra / main-line-gauge look we are building. Good for the door rig and the overall massing. Reshape toward the Aventra proportions and the curved cab front. Attribution required. Strip any TfL trade dress (roundel, Johnston, livery) before use. |
| London Underground Jubilee Line Train 1996 Stock | https://embed-3dwarehouse-classic.sketchup.com/model/6ff4eeaf7dc6f6b861ef2337336d94db/London-Underground-Juiblee-Line-Train-1996-Stock | 3D Warehouse General Model Licence, use in models and renders permitted, re hosting the raw file restricted. NOT repo safe. | SketchUp / Collada | The deep-tube profile, which is what we are NOT building. Kept only as a contrast reference. Do not ship or commit. Likely carries roundel/Johnston detailing. |
| Bombardier S Train Carriage (timblewee) | https://sketchfab.com/3d-models/bombardier-s-train-carriage-london-underground-09c298622ed74c46bd85d6969396943f | CC-BY 4.0 | glTF, rigged doors | Single-carriage version. Same caveats. |
| FREE // Subway Station & R46 Subway (Xlay3D) | https://sketchfab.com/3d-models/free-subway-station-r46-subway-ae5aadde1c6f48a19b32b309417a669b | CC-BY 4.0 | glTF | Includes an R46 NYC subway car. US car, box body, not a tube profile, but useful for interior longitudinal-seating layout and for the train-in-platform-slot massing. |
| Wikimedia Commons, Class 345 category | https://commons.wikimedia.org/wiki/Category:British_Rail_Class_345 | Per image, check each | JPEG | Interior and exterior photos of the Aventra: proportion, panel lines, the curved cab front, bogies, wear. The primary photo reference. |
| Wikipedia, British Rail Class 345 | https://en.wikipedia.org/wiki/British_Rail_Class_345 | Text CC-BY-SA | - | Dimensions table (source of the Aventra figures above). |
| Wikimedia Commons, 1996 Stock category | https://commons.wikimedia.org/wiki/Category:London_Underground_1996_Stock | Per image: mix of CC-BY-SA, CC-BY, some PD. Check each file page. | JPEG | Contrast only: the deep-tube shape to avoid. |
| Wikipedia, London Underground 1996 Stock | https://en.wikipedia.org/wiki/London_Underground_1996_Stock | Text CC-BY-SA | - | Dimensions table for the contrast section below. |
| Wikipedia, loading gauge | https://en.wikipedia.org/wiki/Loading_gauge#Great_Britain | Text CC-BY-SA | - | Background on the British main-line loading gauge the Aventra is built to. |

## Gap

No CC0 or clearly-free Class 345 Aventra model exists. Plan: model the hero
train from the dimensions and profile description above, or take the CC-BY
Bombardier S Stock model (a near-vertical box body, the right family), reshape
it toward the Aventra proportions and the curved wraparound cab front, and
retexture it with the original, non-trade-dress livery (charcoal bodyshell,
sodium cab band, violet door surrounds, made-up operator mark).

## For contrast: the Jubilee line 1996 Stock (what we are NOT building)

Recorded only so a modeller knows what to avoid. The real Jubilee-line platform
at Canary Wharf runs 1996 Stock: deep-level tube stock built to fit a bored
tunnel of roughly 3.8 m diameter, so the body is small and curved.

- Car length about 18 m, body width 2.629 m at waist height (narrowing above and
  below), 2.875 m rail to roof, 126.5 m as a 7-car train.
- 4 double-leaf sliding doors per side per car.
- **NOT walk-through**: separate cars with a connecting door and a short gangway,
  so from the platform you see distinct car bodies with a black articulation gap
  between them.
- Cross-section close to a circle flattened top and bottom, hard tumblehome
  curving in at knee height, doors curved in section, splayed diagonal seats at
  the car ends where the body tapers.
- Full-height platform screen doors at Canary Wharf Jubilee.

The game's train is none of this. It is the tall near-vertical Aventra box
described above. If a texture, proportion or detail choice pulls toward "low fat
rounded worm", it is wrong.
