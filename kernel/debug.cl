void kernel render(global uchar4* texture) {

    int x = get_global_id(0);
    int y = get_global_id(1);
    int w = get_global_size(0);
    int h = get_global_size(1);

    float3 pixel = {0.9f, 0, 0};

    texture[x + y * w].zyx = convert_uchar3(pixel * 255);

}
