# Materials and surfaces

A practical list for the UE5 art pass. Grouped by where it lives in the station.
Every texture source below is CC0 unless noted, so it can be committed. Build the
material from the maps, do not expect a ready made "Tube platform" material.

## Concrete

The dominant material family across both real Canary Wharf stations. Three
distinct concrete reads. Because the game's arena is modelled on the main-line /
Elizabeth-line-scale platform, read 3 (GFRC) is the **default platform wall
treatment**; reads 1 and 2 are the Jubilee-box look, still useful for the track
wall, back-of-house, the concourse and columns.

1. **Board marked in situ concrete**, walls and soffit. Vertical timber plank
   impressions, tie rod holes on a grid, subtle pour lines and colour
   variation between lifts, occasional honeycombing and blemish. Smooth but not
   polished. Mid grey, slight warm cast.
2. **Fair faced structural concrete**, the big oval columns. Same mix, cleaner,
   fewer marks, sometimes a very light seal or anti graffiti coat that adds a
   faint sheen and darkens it.
3. **Sprayed / GFRC panel concrete** (the main-line platform default). Creamy
   pale, near seamless, coved corners, fine matte texture, panel joint lines on
   a large module with a shadow gap.

Wear on concrete: greasy dark hand height smears near openings and stairs,
scuffs and black rubber transfer marks along skirting height where trolleys and
feet hit, water staining and efflorescence streaks below any joint or leak,
chewing gum black dots on horizontal surfaces, drill patch repairs, cable clip
scars, old fixing holes filled a slightly wrong colour.

CC0 texture sources:

- ambientCG concrete category: https://ambientcg.com/list?category=Concrete
  Good starting IDs: Concrete010, Concrete012, Concrete015, Concrete020,
  Concrete033 (formwork boards), PavingStones for platform slabs. All CC0.
- ambientCG bare concrete search: https://ambientcg.com/list?q=concrete+bare
- Poly Haven textures, concrete: https://polyhaven.com/textures/concrete CC0.
  concrete_wall_008, concrete_floor_worn_001, dirty_concrete are close.
- cc0-textures.com (mirror of ambientCG): https://cc0-textures.com/
- TextureCan concrete, CC0: https://www.texturecan.com/category/Concrete/

## Metal

- **Stainless steel**, brushed satin finish, on balustrades, handrails, kiosk
  fronts, escalator side panels and skirts, cladding trims, lift shafts. Fine
  horizontal brush direction, low reflectivity, smudged with fingerprints at
  hand height, polished bright where hands actually grip.
- **Escalator treads and combs**, cast aluminium, ridged, dull, with a yellow
  painted demarcation line worn on the leading edge of each step.
- **Galvanised steel and painted steel** on service doors, riser panels, cable
  tray, walkway grating in back of house. Grey or the palette violet as
  intervention colour.
- **Perforated metal** acoustic ceiling baffles and the metal acoustic tubes
  across the Jubilee soffit. Semi gloss dark grey or off white.
- **Cast iron tunnel segments** visible in the running tunnels and any exposed
  tunnel mouth: bolted rings, grease, black, cable brackets on every ring.

Wear on metal: bright polished grip zones on poles and rails, fingerprint haze
on flat stainless, rust bleed at fixings and cut edges, scratched signage of
scuffed kick plates, dull dust film on horizontal ledges.

CC0 texture sources:

- ambientCG metal category: https://ambientcg.com/list?category=Metal
  Metal032 (brushed), Metal009, MetalPlates006, MetalWalkway sets. CC0.
- ambientCG "corrugated" and "perforated" queries for baffle and grating.
- Poly Haven metal: https://polyhaven.com/textures/metal CC0.

## Tile

Canary Wharf Jubilee is largely concrete, not the classic bevelled tile of
older Tube stations. But for a fictionalised station you may want a legacy tiled
section (older cross passage, a WC, a back corridor) to add texture:

- **Biscuit / cream rectangular wall tile**, brick bond, thin grout, in old Tube
  passages. Crazed glaze, chipped corners, replaced tiles in the wrong shade,
  grime in the grout lines.
- **Small hexagonal or square mosaic floor tile** in station WCs and old
  entrances, worn smooth in the walking line.
- Avoid reproducing any specific historic station's tile pattern that is a
  recognised heritage design; a plain cream brick bond is generic.

CC0 texture sources:

- ambientCG tiles: https://ambientcg.com/list?category=Tiles Tiles074, Tiles093
  (subway brick bond), Tiles101. CC0.
- Poly Haven, "subway tile": https://polyhaven.com/textures search subway. CC0.

## Floor treatments

- **Platform floor**: large format grey terrazzo or ground concrete paving
  slabs, matte, with a broad walking line polished lighter down the middle and
  scuffed darker at the platform edge. Occasional cracked or lifted slab.
- **Concourse floor**: same family, larger slabs, expansion joints on a grid,
  drainage channels and slot drains near entrances, tracked in rain water and
  grit in a fan shape inside every doorway.
- **Yellow safety line** set back from the platform edge, painted or inlaid
  strip, always worn and patchily repainted. Describe and recreate as an
  original mark, do not copy TfL's exact spec artwork.
- **Back of house**: sealed screed, painted concrete, or steel plate, with
  yellow and black hazard edging on steps and level changes.
- **Nosings**: contrasting stair nosings on every step, metal or resin, worn to
  bare metal on the leading edge.

CC0 texture sources:

- ambientCG PavingStones: https://ambientcg.com/list?q=paving CC0.
- ambientCG "terrazzo": https://ambientcg.com/list?q=terrazzo CC0.
- Poly Haven floor: https://polyhaven.com/textures/floor CC0.

## Tactile paving (UK standard, not TfL IP)

Geometry is set by UK government guidance, so it is safe to reproduce exactly.

- **Platform edge (off street) warning surface**: offset rows of flat topped
  domes, **5 mm high (plus or minus 0.5 mm)**, **66.5 mm centre to centre**, in
  an 800 mm deep band along the full platform edge, set back behind the yellow
  line. This is the "lozenge" blister, different from the street blister.
- **Blister surface** (top and bottom of external steps, crossings): flat topped
  domes **~25 mm diameter, 5 mm high**, in straight rows, on 400 x 400 mm slabs.
- **Corduroy hazard surface** (top and bottom of internal stairs, level
  changes): rounded bars **6 mm high, 20 mm wide, 50 mm centre to centre**, bars
  run across the direction of travel, 400 x 400 mm module.
- Colour: usually buff or charcoal, high contrast against the surrounding floor.

Model these as a tiling normal and height map on a 400 mm tile, plus a dedicated
platform edge strip mesh. Sources: ambientCG has a "tactile paving" search;
otherwise author from the dimensions above.

- GripClad regulations summary: https://gripclad.co.uk/useful-information/tactile-paving-regulations/
- Designing Buildings wiki, hazard warning surfaces: https://www.designingbuildings.co.uk/wiki/Hazard_warning_surfaces
- Tobermore specify guide: https://www.tobermore.co.uk/professional/blog/tactile-paving-how-to-specify-corduroy-and-blister-paving-for-public-realm-safety/

## Handrails and balustrades

Design language at Canary Wharf Jubilee:

- **Stainless steel tube handrail**, ~40 to 50 mm diameter, continuous, radiused
  returns into the ground or wall at the ends, no sharp terminations.
- **Glass balustrade infill** in stainless shoe channel or with disc fixings,
  toughened, edges polished, often slightly greened, fingerprinted, dusty at the
  base channel.
- **Escalator balustrade**: glass panels with a moving rubber handrail on a
  stainless capping, brush skirt at deck level, stainless skirt panels below.
- Back of house: plain painted steel tube handrail, mid grey, chipped to primer
  and rust at the welds.
- Second, waist high rail runs are common as queue guidance and crowd control:
  stainless posts with a single or double rail, or removable belt barrier posts.

## Lighting hardware (as surfaces to model)

- Recessed linear luminaires in the concrete soffit coffers and along the
  acoustic baffle runs. Long, dim, cool white in the real station.
- Uplighters washing the oval columns.
- Bulkhead and batten fittings in back of house, some flickering or dead for the
  survival dressing.
- Platform edge screen door head has an integrated light line and signage box.
- Emergency and running rail level lighting: low blue and white markers along
  the trackbed.

## Wear pattern summary (where hands and feet actually wear a station)

| Zone | What wears | Read |
|---|---|---|
| Handrails, poles, escalator rubber | polished bright, grease sheen | lightest, glossiest spots |
| Wall at 900 to 1100 mm | hand smears, shoulder rub, bag scuffs | dark greasy band |
| Floor centreline of every route | abraded lighter, gum, flattened | pale worn trail |
| Doorway thresholds and gatelines | grit fan, water stain, sole polish | dark then pale fan shape |
| Platform edge | scuff, yellow line loss, tactile dome wear | most beaten up strip in the station |
| Stair nosings | worn to bare metal on the front edge | bright line on each step |
| Bench seat and back | polished, gum underneath, litter behind | localised polish plus rubbish |
| Corners and column bases | trolley and cleaner machine impacts | chips, black rubber transfer |
| Ceiling above lights and vents | soot and dust plume | grey staining upward |
| Any joint, leak or duct | water streak, efflorescence, rust bleed | vertical stain below |

## Palette to key materials to (from CLAUDE.md)

`#16161C` charcoal, `#6C4C9C` violet, `#E0A030` sodium, `#B02030` crimson.
Concrete and steel are near neutral; push the violet into intervention elements
(new barriers, signage bars, wayfinding), the sodium into lighting and hazard
marking, the crimson into blood and emergency equipment, charcoal into shadow
and grime.
