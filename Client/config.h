#pragma once

const std::string game_prc_settings = R"(
# --------------  Development options  --------------
    pstats-gpu-timing #t
    gl-debug #t
    gl-debug-object-labels #t

# -------------- Production options ---------------

    # pstats-gpu-timing #f
    # gl-debug #f
    # gl-debug-object-labels #f

# ----------------- Misc Settings -----------------

    window-title My Game
    win-size 1280 720
    fullscreen #f
    sync-video #f

    show-frame-rate-meter #t
    pstats-max-rate 200

    load-display pandagl 
    aux-display pandadx9
    audio-library-name p3openal_audio

    framebuffer-multisample #t
    multisamples 4
    
    support-stencil #f
    framebuffer-stencil #f

    textures-power-2 none 
    compressed-textures #t
    driver-compress-textures #t  
    preload-textures #t
    preload-simple-textures #t  
    allow-incomplete-render #t 
    text-flatten 0
    text-dynamic-merge 1

    vfs-case-sensitive #t

    state-cache #t
    transform-cache #t

    model-cache-dir $USER_APPDATA/MyGame/cache  
    model-cache-bam #t      

    audio-library-name p3openal_audio   
    audio-active #t    
    audio-cache-directory $USER_APPDATA/MyGame/audio_cache    
    
    threads 4   
    threads-per-core 1   
    loader-num-threads 2  
    max-threads 6  
    loader-thread-priority normal   

    pstats-host localhost 
    pstats-port 9099

    notify-level-pnmimage error
    interpolate-frames #t

    gl-force-fbo-color #f
    gl-validate-shaders #f
    gl-skip-shader-recompilation-warnings #t

    always-store-prev-transform #t
    allow-incomplete-render #t
)";