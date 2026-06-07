use zensim::{RgbSlice, Zensim, ZensimProfile};

fn gray_to_rgb_pixels(gray: &[u8]) -> Vec<[u8; 3]> {
    gray.iter().map(|&g| [g, g, g]).collect()
}

fn channels_to_rgb_pixels(r: &[u8], g: &[u8], b: &[u8]) -> Vec<[u8; 3]> {
    r.iter()
        .zip(g.iter())
        .zip(b.iter())
        .map(|((&r, &g), &b)| [r, g, b])
        .collect()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn zensim_score_gray(
    a: *const u8,
    b: *const u8,
    width: u32,
    height: u32,
    out_score: *mut f64,
) -> i32 {
    if a.is_null() || b.is_null() || out_score.is_null() {
        return -1;
    }

    let len = width as usize * height as usize;
    let slice_a = unsafe { std::slice::from_raw_parts(a, len) };
    let slice_b = unsafe { std::slice::from_raw_parts(b, len) };

    let rgb_a = gray_to_rgb_pixels(slice_a);
    let rgb_b = gray_to_rgb_pixels(slice_b);

    let z = Zensim::new(ZensimProfile::latest());
    let w = width as usize;
    let h = height as usize;
    let source = RgbSlice::new(&rgb_a, w, h);
    let distorted = RgbSlice::new(&rgb_b, w, h);

    match z.compute(&source, &distorted) {
        Ok(result) => {
            unsafe {
                *out_score = result.score();
            }
            0
        }
        Err(_) => -1,
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn zensim_score_rgb(
    a_r: *const u8,
    a_g: *const u8,
    a_b: *const u8,
    b_r: *const u8,
    b_g: *const u8,
    b_b: *const u8,
    width: u32,
    height: u32,
    out_score: *mut f64,
) -> i32 {
    if a_r.is_null()
        || a_g.is_null()
        || a_b.is_null()
        || b_r.is_null()
        || b_g.is_null()
        || b_b.is_null()
        || out_score.is_null()
    {
        return -1;
    }

    let len = width as usize * height as usize;
    let slice_a_r = unsafe { std::slice::from_raw_parts(a_r, len) };
    let slice_a_g = unsafe { std::slice::from_raw_parts(a_g, len) };
    let slice_a_b = unsafe { std::slice::from_raw_parts(a_b, len) };
    let slice_b_r = unsafe { std::slice::from_raw_parts(b_r, len) };
    let slice_b_g = unsafe { std::slice::from_raw_parts(b_g, len) };
    let slice_b_b = unsafe { std::slice::from_raw_parts(b_b, len) };

    let rgb_a = channels_to_rgb_pixels(slice_a_r, slice_a_g, slice_a_b);
    let rgb_b = channels_to_rgb_pixels(slice_b_r, slice_b_g, slice_b_b);

    let z = Zensim::new(ZensimProfile::latest());
    let w = width as usize;
    let h = height as usize;
    let source = RgbSlice::new(&rgb_a, w, h);
    let distorted = RgbSlice::new(&rgb_b, w, h);

    match z.compute(&source, &distorted) {
        Ok(result) => {
            unsafe {
                *out_score = result.score();
            }
            0
        }
        Err(_) => -1,
    }
}
