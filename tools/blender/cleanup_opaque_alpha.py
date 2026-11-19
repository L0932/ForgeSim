import bpy
from array import array

DRY_RUN = False
alpha_cache = {}


def has_white_alpha(image):
    key = image.as_pointer()
    if key in alpha_cache:
        return alpha_cache[key]

    result = False

    # Only inspect ordinary static images, not movies or UDIM tiles.
    if image.source == 'FILE':
        try:
            count = len(image.pixels)
            width, height = image.size

            if width > 0 and height > 0 and count == width * height * 4:
                pixels = array('f', [0.0]) * count
                image.pixels.foreach_get(pixels)

                # Exact comparison: preserve even slight transparency.
                result = all(
                    pixels[i] == 1.0
                    for i in range(3, count, 4)
                )
        except Exception as error:
            print(f"Skipped image {image.name}: {error}")

    alpha_cache[key] = result
    return result


matches = 0

for material in bpy.data.materials:
    if not material.use_nodes or not material.is_editable:
        continue

    tree = material.node_tree

    # Leave animated node trees untouched.
    if tree.animation_data:
        continue

    for node in tree.nodes:
        if node.type != 'BSDF_PRINCIPLED':
            continue

        alpha = node.inputs.get("Alpha")
        if alpha is None or len(alpha.links) != 1:
            continue

        link = alpha.links[0]
        texture = link.from_node

        # Only direct Image Texture Alpha → Principled Alpha links.
        if (
            texture.type != 'TEX_IMAGE'
            or link.from_socket != texture.outputs.get("Alpha")
            or texture.image is None
            or texture.extension == 'CLIP'
        ):
            continue

        if not has_white_alpha(texture.image):
            continue

        matches += 1
        print(
            f"{'Would fix' if DRY_RUN else 'Fixed'}: "
            f"{material.name} / {node.name} / {texture.image.name}"
        )

        if not DRY_RUN:
            tree.links.remove(link)
            alpha.default_value = 1.0

print(f"{matches} matching connections. DRY_RUN={DRY_RUN}")