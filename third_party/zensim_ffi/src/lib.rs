use zensim::{RgbSlice, Zensim, ZensimProfile};

fn gray_to_rgb_pixels(gray: &[u8]) -> Vec<[u8; 3]> {
    gray.iter().map(|&g| [g, g, g]).collect()
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
