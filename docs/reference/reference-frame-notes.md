# Reference frame

The single visual target for LAST TRAIN. `reference-frame.png` in this folder is
the frame every art and lighting decision is measured against. Keep it open while
building. It is a Phase 7 target, not a Phase 4 one: do not measure the grey box
against it.

`reference-frame.png` shows a first person shooter on a fictionalised London
Underground platform. A train fills the entire right side of frame as a wall. The
platform recedes left to a vanishing point down a tiled corridor. A horde of
zombies arrives down that corridor toward the camera. The player is boxed in on
the train side and open on the platform side. Warm sodium and white functional
lighting, violet line accent on the rolling stock and furniture, crimson
emergency lighting receding into the tunnel as a depth cue. Wet reflective floor.
A departure board hangs from the ceiling mid platform. Restrained modern shooter
HUD furniture in the corners.

---

## 1. What to lift

**Composition.** Train as a full height wall down one side, platform receding to
a vanishing point, horde funnelled down the corridor toward you, one side closed
and one side open. This is the default framing for every platform in the game.
Canary Wharf's grey box blockout in Phase D should already read this way.

**Palette.** Sodium amber and warm white for function. Violet as the line accent,
sparingly, mostly rolling stock and signage furniture. Crimson for emergency
lighting only, receding down the tunnel so it reads as depth. Everything else
desaturated concrete, steel and dirty tile. Saturation is a resource, spent on
those three accents.

| Role | Hex |
|---|---|
| Charcoal base | `#16161C` |
| Violet accent | `#6C4C9C` |
| Sodium amber | `#E0A030` |
| Crimson | `#B02030` |

**Materials.** Wet platform floor with real reflections, which Lumen gives almost
free and which carries more of the look than any other single decision. Dirty
tile at head height, painted concrete above, brushed steel furniture, rubber
tactile paving. Wear concentrated where people touch things.

**Emergency lighting as a depth cue.** The red units are not general
illumination. They are a gradient down the tunnel telling you where the space
continues. Copy this precisely. Do not make the whole map red.

**Dressing.** Litter drifts in corners, not scattered evenly. A bin. A bench run.
Abandoned personal items as environmental storytelling. Fluid staining near the
horde, not everywhere.

## 2. The best idea in the frame is a mechanic

The departure board. In the frame it shows two services with minutes and a clock.
That is the train timer made diegetic. Do not put the train countdown in the HUD.
Put it on the board, on the platform displays, and in the announcements. The
player learns to read the station rather than a widget. Built in Phase C.

## 3. What must not be copied

The frame is full of protected material. Every art task must restate these or
they creep back in.

| In the frame | Status | Substitute |
|---|---|---|
| Roundel on the platform wall | TfL trademark | Original station mark: a violet horizontal bar over a charcoal field, station name in the project typeface. No circle and bar |
| Johnston wayfinding type | Licensed typeface | An original or freely licensed geometric sans with a taller x-height. Not a Johnston clone |
| "Way out" panel in house style | Trademark dress | Same information, original panel geometry and colour split |
| Official line diagram | Copyright | Original network schematic, which doubles as the HUD map substitute |
| Elizabeth line livery on the train | Operator identity | Original livery in the project palette. Violet is fine, the specific stripe arrangement is not |
| Line name on signage | Trademark | Rename the line in world. Station names stay factual |
| `SAUG 9MM` and the CoD HUD furniture | Another game's names and style | Original weapon names throughout, restrained classic HUD per Phase G |
| Round counter, challenge tracker, minimap, kill feed, exfil banner | Modern shooter HUD, banned by the style guide | Round, points, health, ammo, perks only. No permanent minimap, no challenge tracker, no kill feed, no exfil banner |

**Fair game.** Station names and geography, which are factual. Panel shapes and
mounting heights. Sign proportions. Tactile paving patterns. Escalator geometry
and pitch. Tunnel ring spacing. Platform edge treatment. Barrier forms and gate
mechanisms. Bench and bin design language. Station architecture generally.
Copying this closely is what makes the place feel real.

## 4. How the frame maps to the phases

| Frame element | Where it gets built | Notes |
|---|---|---|
| Platform composition, train as a wall, corridor to a vanishing point | Phase D grey box | Must read right in primitives before any art |
| Train arriving, dwelling, doors, boarding | Phase C | `ALTTrainActor` |
| Departure board | Phase C | Diegetic timer, text render component |
| Horde funnelled down the platform | Phase B, D | Flow field plus spawn routes down the tunnel mouths |
| Wet floor, Lumen reflections, sodium lighting, volumetric tunnel haze | Phase F | Fable led, post process volume plus Lumen tuning |
| Violet station identity, original signage, original livery, original ads | Phase F | Original identity only, see the table above |
| HUD | Phase G | Restrained, explicitly not the frame's furniture |
