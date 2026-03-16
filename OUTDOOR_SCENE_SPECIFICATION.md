# Outdoor Maze Runner Scene - Ray-Traced Render Specification

## Overview
This document specifies the requirements for creating a realistic, ray-traced 3D render of a maze-runner style game scene set outdoors. The render should showcase key gameplay features including collectible stacking mechanics, improved path navigation, and realistic outdoor lighting with both sun and moon visible.

## Scene Goals and Key Features

### 1. Stackable Giftbox Collection System

#### Visual Requirements
- **Stack Size**: Player holds 6–12 giftboxes stacked vertically
- **Box Positioning**: Boxes snap neatly on top of each other, aligned and stable
- **Stack Coherence**: Despite natural sway from walking, the stack maintains believability
- **Box Variations**:
  - Slight size variations (±5-10% scale difference)
  - Different wrapping paper patterns:
    - Polka dots
    - Stripes (horizontal, vertical, diagonal)
    - Solid colors with subtle texture
    - Plaid/checkered patterns
  - Varied ribbon colors: red, gold, silver, blue, green
  - Crisp geometry on ribbons and box edges

#### Animation Details
- **Natural Sway**:
  - Stack tilts 2-4 degrees side-to-side with walk cycle
  - Subtle forward/backward tilt (1-2 degrees) with stride
  - Top boxes show more movement than bottom (dampened spring effect)
- **Box Addition**:
  - New box appears 20cm above stack top
  - Drops gently onto stack over 0.3 seconds
  - Slight bounce/settle effect on landing

#### Physical Accuracy
- Boxes show realistic mass distribution
- Stack center of gravity remains believable
- No interpenetration between boxes
- Contact shadows between stacked boxes

### 2. Widened Walking Path

#### Path Dimensions
- **Current Width**: Standard maze corridor (approximately 2 meters)
- **New Width**: 4 meters (2x increase)
- **Transition Zones**: Smooth 1-meter transitions at intersections

#### Path Surface Details
- **Base Material**: Choose one:
  - **Option A - Packed Dirt**:
    - Earth-tone browns (hex #8B7355 to #A0826D)
    - Subtle micro-roughness (0.7-0.8 roughness value)
    - Small embedded stones (3-8cm diameter)
  - **Option B - Cobblestone**:
    - Weathered stone gray (hex #7A7D7D to #9BA3A8)
    - Individual stone definition (20-30cm squares)
    - Worn edges, subtle height variation (±1cm)
  - **Option C - Gravel**:
    - Mixed gray/tan aggregate (hex #B8B8AA)
    - Individual pebbles (0.5-2cm visible)
    - Looser material at edges

#### Path Wear Patterns
- **Center Wear**: Darkened, compacted 1.5m wide strip down center
- **Footprints**:
  - Random placement, various depths
  - Concentrated in high-traffic areas
  - 5-10 visible footprints per 3-meter section
- **Edge Definition**:
  - Slight elevation drop at path edges (2-3cm)
  - Scattered gravel/debris at boundaries
- **Scuff Marks**: Drag marks from boxes, random scuffs

#### Navigation Features
- Clean sight lines down path (minimum 5-meter visibility)
- Subtle center-line wear to guide navigation
- No obstacles in main walkway

### 3. Maze Wall Structure

#### Wall Type
- **Recommendation**: Tall hedge walls for outdoor authenticity
- **Alternative Options**:
  - Weathered stone walls with moss
  - Wooden fence panels with vines

#### Hedge Wall Specifications (Primary Option)
- **Height**: 2.5-3 meters (above player eye level)
- **Thickness**: 40-60cm
- **Foliage Density**: Dense, no see-through gaps
- **Species Appearance**: Box hedge or yew
- **Color Palette**:
  - Deep greens (hex #2C5F2D to #4A7C59)
  - Slight yellow-green highlights on sun-facing sides
  - Darker shadows in crevices (hex #1A3D1F)
- **Surface Detail**:
  - Individual leaf clusters visible at close range
  - Slight irregularity (not perfectly trimmed)
  - Small gaps/holes sparingly (< 1% of surface)
  - Subtle windswept appearance

#### High-Frequency Detail Requirements
- Sub-centimeter leaf detail where visible
- Procedural variation across wall surfaces
- Realistic branching structure visible in gaps
- Accumulated leaves/debris at wall base

### 4. Sky and Celestial Bodies

#### Time of Day
- **Recommended Time**: Late afternoon (approximately 4:30 PM)
- **Season**: Early autumn (warm light, no harsh summer glare)
- **Weather**: Clear with minimal cloud cover

#### Sun Specifications
- **Position**: 20-25 degrees above horizon, southwest direction
- **Appearance**:
  - Brilliant white-yellow core (hex #FFFACD)
  - Warm yellow halo (hex #FFD700)
  - Subtle lens flare if looking near sun direction
- **Light Quality**:
  - Color temperature: 4500K (warm daylight)
  - Intensity: Full daylight but softening toward evening
  - Long shadow length (3-4x object height)

#### Moon Specifications
- **Position**: 45-50 degrees above horizon, southeast direction
- **Phase**: Waxing gibbous (70-80% illuminated)
- **Appearance**:
  - Pale gray-blue (hex #D3D3E3)
  - Subtle surface detail (maria visible)
  - Faint halo in daylit sky
  - Apparent diameter: 0.5 degrees (realistic)
- **Visibility**: Clearly visible but not dominant, natural daylight moon appearance

#### Sky Color Gradient
- **Zenith**: Deep blue (hex #4A90E2)
- **Mid-Sky**: Lighter blue (hex #7BB3E8)
- **Horizon**:
  - Warm transition zone: peachy-orange (hex #FFB088)
  - Near sun: brighter warm tones (hex #FFCC99)
  - Opposite direction: cooler blues (hex #B0C4DE)

#### Atmospheric Effects
- **Aerial Perspective**:
  - Objects fade toward sky color with distance
  - Effective range: begins at 20m, full effect at 50m
- **Atmospheric Scattering**:
  - Rayleigh scattering evident (blue sky)
  - Mie scattering near horizon (haziness)
- **Volumetric Lighting**:
  - Subtle god rays if sun partially occluded by walls
  - Light shafts through gaps in foliage (if present)
  - Atmosphere density: 0.02-0.03 (subtle but visible)

## Lighting and Ray Tracing Requirements

### Global Illumination
- **Technique**: Full path tracing with minimum 64 samples per pixel
- **Bounces**: Minimum 4 light bounces, 6-8 recommended
- **Indirect Lighting**:
  - Ground plane reflects warm tones onto nearby objects
  - Hedge walls cast subtle green color bleed onto path
  - Skylight provides soft ambient from all directions

### Shadows
- **Primary Shadows**:
  - Sharp penumbra from sun (solar disc 0.5° apparent size)
  - Soft umbra transition (physically accurate)
  - No banding or stepping in shadow gradients
- **Self-Shadowing**:
  - Ambient occlusion in crevices and contact points
  - Micro-shadow detail on foliage and ground texture
- **Transparency Shadows**:
  - If foliage has transparency, cast dappled shadows
  - Colored transmission through thin leaves

### Reflections and Refractions
- **Giftbox Ribbons**:
  - Glossy reflections (0.1-0.2 roughness)
  - Metallic sheen on foil ribbons (metallic = 0.8-1.0)
  - Accurate fresnel reflections
- **Ground Surface**:
  - Minimal reflectivity unless wet patches included
  - If cobblestone: slight glossiness on worn stones (0.6-0.7 roughness)
- **Puddles (Optional Enhancement)**:
  - Perfect mirror reflection with slight ripple distortion
  - Clear sky and sun reflection
  - Realistic refraction at water-air boundary

### Material PBR Properties

#### Giftbox Wrapping Paper
- **Base Color**: Various per box (see variations above)
- **Roughness**: 0.3-0.5 (slight sheen)
- **Metallic**: 0.0 (unless metallic wrapping, then 0.7-0.9)
- **Normal Map**: Embossed texture details (0.02mm depth)

#### Ribbons
- **Satin Ribbon**:
  - Roughness: 0.2-0.3
  - Anisotropic reflection along ribbon length
  - Subsurface scattering through thin areas
- **Foil Ribbon**:
  - Metallic: 0.9-1.0
  - Roughness: 0.1
  - Sharp specular highlights

#### Hedge Walls
- **Base Color**: Varied greens (see above)
- **Roughness**: 0.8-0.9 (matte foliage)
- **Subsurface Scattering**:
  - Enabled for thin leaves
  - Transmission distance: 0.5-1cm
  - Greenish transmission color
  - Backlit leaves show warm glow

#### Path Surface
- **Dirt**:
  - Roughness: 0.75
  - Subtle color variation (±5% HSV value)
  - Displacement map (0.2-1cm amplitude)
- **Cobblestone**:
  - Roughness: 0.5-0.7 (varied per stone)
  - Displacement: 0.5-1.5cm per stone
  - Grout lines: darker, more rough (0.9 roughness)

### Volumetric Atmosphere
- **Density**: 0.02-0.03 (subtle haze)
- **Scattering**:
  - Rayleigh: RGB (0.0054, 0.0135, 0.0331)
  - Mie: 0.003 (forward scattering)
- **God Rays**:
  - Enabled when sun rays intersect hedge gaps
  - 5-10cm wide shafts
  - Subtle intensity (not overdramatic)

## Camera and Composition

### Camera Specifications
- **Type**: Third-person over-the-shoulder
- **Lens**: 35-50mm equivalent (38° to 55° horizontal FOV)
- **Position**:
  - 1.8-2.0 meters behind player
  - 0.3-0.5 meters to player's right (looking over right shoulder)
  - 1.5-1.6 meters above ground (slightly above player eye level)
- **Look-At Point**: Player's upper back / giftbox stack top
- **Camera Tilt**: 0-2 degrees downward (neutral to slight downward gaze)

### Depth of Field
- **Focus Point**: Giftbox stack (sharp focus)
- **Aperture**: f/4.0 (natural, not excessive bokeh)
- **Bokeh Quality**: Circular, smooth gradients
- **Focus Range**:
  - In-focus: 1-4 meters from camera
  - Gradual falloff: 4-8 meters
  - Full blur: beyond 8 meters (path ahead softens)

### Framing and Rule of Thirds
- **Player Position**: Right third of frame (rule of thirds)
- **Giftbox Stack**: On right vertical third line, prominent
- **Path Ahead**: Left and center frame, showing depth
- **Sky**: Upper third, showing sun and moon
- **Horizon Line**: Slightly below middle (natural perspective)

### Composition Balance
- **Foreground**: Player and giftboxes (sharp, detailed)
- **Midground**: 3-5 meters of path, nearby walls (in focus)
- **Background**: Path extending into distance (soft focus)
- **Leading Lines**: Path edges guide eye into frame depth
- **Negative Space**: Sky and distant path provide breathing room

## Environment Detail

### Ground Scatter Objects
- **Small Debris** (per 10 square meters):
  - 3-5 small stones (1-3cm)
  - 2-4 leaves (fallen from walls)
  - 1-2 small sticks/twigs
- **Placement**: Random but natural, avoiding center walkway
- **Accumulation**: More debris near wall edges

### Atmospheric Props (Sparingly)
- **Signpost (Optional)**:
  - Weathered wood
  - Simple directional arrow
  - Positioned at path junction edge
  - Not obstructing view
- **Lantern (Optional)**:
  - Wall-mounted on hedge/stone
  - Decorative, unlit in daylight
  - Wrought iron style
- **Fallen Leaves**:
  - Small clusters (5-10 leaves)
  - 2-3 clusters visible
  - Autumn colors if seasonal

### Distance Details
- **Path Continuation**: Visible 8-12 meters ahead
- **Wall Texture Fading**: High detail nearby, procedural simplification at distance
- **Far Path Details**: Subtle but present (maintains realism)

## Quality and Technical Specifications

### Resolution and Sampling
- **Output Resolution**: 4K (3840 x 2160 pixels) minimum
- **Sampling**:
  - 64-128 samples per pixel for path tracing
  - Adaptive sampling where beneficial
  - Denoising: High-quality AI denoiser (OptiX/OIDN)
- **Render Time**: Accept longer render (hours) for quality

### Post-Processing
- **Tone Mapping**:
  - Filmic/ACES curve
  - Preserve highlight detail (sun disc, bright sky)
  - Natural contrast (1.1-1.2 gamma)
- **Color Grading**:
  - Warm late-afternoon look
  - Slightly boost oranges/yellows (sun warmth)
  - Desaturate distant elements (atmospheric perspective)
- **Bloom**:
  - Subtle bloom on sun and bright highlights
  - 1-2% screen area affected
  - Natural glow, not overdone
- **Vignette**: Very subtle or none (natural lens falloff only)
- **Chromatic Aberration**: Minimal or none (realistic lens quality)
- **Lens Flare**: Optional, subtle if included (sun interaction)

### Texture Quality
- **4K Textures**: All hero objects (giftboxes, nearby walls)
- **2K Textures**: Midground elements (path, distant walls)
- **Tiling Prevention**: Use texture blending and variation
- **Normal Maps**: 2K-4K for surface detail
- **Displacement**: Real geometry where visible, baking otherwise

### Geometric Complexity
- **Giftboxes**:
  - 5,000-10,000 polygons per box (with ribbon detail)
  - 12 boxes = 60,000-120,000 polygons total
- **Hedge Walls**:
  - High-density mesh (500,000+ polygons per 10m section)
  - Instanced leaf geometry for efficiency
- **Path**:
  - Displacement detail (1-2 million polygons for visible area)
  - Tessellation for micro-detail

### Color Accuracy
- **Color Space**: Linear workflow, sRGB output
- **Exposure**: Natural, no clipped whites (except sun core)
- **White Balance**: 4500K (warm daylight)
- **Dynamic Range**: Maintain detail in shadows and highlights
- **No Over-Saturation**: Natural colors, avoid cartoon look

## Prohibited Elements (Negative Prompt)

### Visual Style Prohibitions
- ❌ No cartoon or toy-like appearance
- ❌ No cel-shading or non-photorealistic rendering
- ❌ No flat lighting (must have depth and shadow)
- ❌ No blurry or low-resolution textures
- ❌ No low-poly geometric simplification (where visible)
- ❌ No over-saturated or unnatural colors
- ❌ No incorrect shadow directions (must match sun)

### Technical Prohibitions
- ❌ No visible noise or fireflies
- ❌ No banding in gradients
- ❌ No aliasing or jagged edges
- ❌ No texture seams or tiling artifacts
- ❌ No incorrect light behavior (physically implausible)

### Anatomical Prohibitions
- ❌ No duplicate limbs or hands on player
- ❌ No malformed player proportions
- ❌ No unnatural joint angles or poses

### UI and Overlays
- ❌ No text overlays
- ❌ No watermarks
- ❌ No UI elements or HUD
- ❌ No debug visualization
- ❌ No frame borders or matting

## Player Character Specifications

### Appearance
- **Style**: Realistic human proportions, game character aesthetic
- **Clothing**: Casual outdoor attire (jacket, pants, hiking boots)
- **Build**: Average build, athletic stance
- **Posture**: Mid-stride walking pose, natural arm swing
- **Hair**: Natural movement, slight windswept

### Animation
- **Walk Cycle**: Frame 12-16 of standard walk (mid-stride)
- **Giftbox Stack Hold**:
  - One hand supporting bottom of stack
  - Other hand stabilizing upper portion
  - Arms positioned naturally
- **Head Angle**: Slightly upward (looking ahead down path)

### Player-Giftbox Interaction
- **Contact Points**: Believable grip on boxes
- **Stack Weight**: Slight lean back to counter weight
- **Natural Effort**: Posture suggests carrying moderate load

## Implementation Notes for Rendering

### Recommended Software
- **Blender Cycles**: Path tracing, excellent PBR materials
- **Unreal Engine 5**: Lumen GI, Nanite for detail, Quixel assets
- **V-Ray**: Professional path tracing, realistic materials
- **Octane Render**: GPU path tracing, excellent outdoor scenes
- **Arnold**: Robust path tracing, industry-standard quality

### Scene Setup Workflow
1. **Block Out**: Establish path, walls, player placement, camera angle
2. **Lighting**: Set sun position, add sky dome, configure GI
3. **Hero Assets**: Model and texture giftboxes with variations
4. **Environment**: Build hedge walls with foliage detail
5. **Ground**: Create path surface with wear patterns
6. **Details**: Add scatter objects, debris, props
7. **Player**: Rig and pose character, integrate giftbox stack
8. **Camera**: Fine-tune position, DOF, framing
9. **Materials**: Configure all PBR properties accurately
10. **Render**: Set sampling, render passes, denoise
11. **Post**: Tone mapping, color grade, final adjustments

### Asset Sources
- **Foliage**: Quixel Megascans, SpeedTree, custom
- **Ground Textures**: Photogrammetry scans, procedural
- **Giftbox Wrapping**: Procedural patterns, texture libraries
- **HDRI**: Polyhaven outdoor HDRIs (4K-8K)

## Deliverable Specifications

### Final Output
- **Format**: PNG or EXR (EXR for further post-processing)
- **Resolution**: 3840 x 2160 (4K) or higher
- **Bit Depth**: 16-bit per channel minimum
- **Color Profile**: sRGB IEC61966-2.1

### Quality Criteria
- ✅ Photorealistic lighting and materials
- ✅ Sharp focus on giftbox stack
- ✅ Natural depth of field
- ✅ Physically accurate shadows
- ✅ Believable atmosphere
- ✅ Clean render (no artifacts)
- ✅ Professional composition
- ✅ Meets all specifications above

---

**Document Version**: 1.0
**Created**: 2026-03-16
**Purpose**: Specification for commissioned ray-traced outdoor maze-runner scene render
