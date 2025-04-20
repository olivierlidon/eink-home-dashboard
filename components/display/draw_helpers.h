void draw_rounded_dithered_box(Display &it, int x_start, int y_start, int width, int height, int radius, Color color) {
  for (int x = x_start; x < x_start + width; x += 2) {
    for (int y = y_start; y < y_start + height; y += 2) {
      bool in_corner = false;

      // Top-left corner
      if (x < x_start + radius && y < y_start + radius) {
        int dx = x - (x_start + radius);
        int dy = y - (y_start + radius);
        in_corner = (dx * dx + dy * dy > radius * radius);
      }

      // Top-right corner
      else if (x >= x_start + width - radius && y < y_start + radius) {
        int dx = x - (x_start + width - radius - 1);
        int dy = y - (y_start + radius);
        in_corner = (dx * dx + dy * dy > radius * radius);
      }

      // Bottom-left corner
      else if (x < x_start + radius && y >= y_start + height - radius) {
        int dx = x - (x_start + radius);
        int dy = y - (y_start + height - radius - 1);
        in_corner = (dx * dx + dy * dy > radius * radius);
      }

      // Bottom-right corner
      else if (x >= x_start + width - radius && y >= y_start + height - radius) {
        int dx = x - (x_start + width - radius - 1);
        int dy = y - (y_start + height - radius - 1);
        in_corner = (dx * dx + dy * dy > radius * radius);
      }

      // Fix for odd x/y starts: consistent dithering regardless of origin
      if (!in_corner && ((x + y - x_start - y_start) % 4 == 0)) {
        it.draw_pixel_at(x, y, color);
      }
    }
  }
}

void wrap_text(Display &it, int x, int y, int width, int line_height,
  const char *text, BaseFont* font,
  Color color_white, Color color_black,
  int current_portion = 0, int lines_to_show = 6) {
  String long_text = text;
  int y_offset = y;

  // Step 1: Split by '\n'
  std::vector<String> paragraphs;
  String paragraph = "";
  for (int i = 0; i < long_text.length(); i++) {
    if (long_text[i] == '\n') {
      paragraphs.push_back(paragraph);
      paragraph = "";
    } else {
      paragraph += long_text[i];
    }
  }

  if (paragraph.length() > 0) {
    paragraphs.push_back(paragraph);
  }

  // Step 2: Wrap each paragraph
  std::vector<String> lines;
  for (const String &para : paragraphs) {
    std::vector<String> words;
    String word = "";

    for (int i = 0; i < para.length(); i++) {
      if (para[i] == ' ') {
        if (word.length() > 0) {
          words.push_back(word);
          word = "";
        }
      } else {
        word += para[i];
      }
    }

    if (word.length() > 0) {
      words.push_back(word); // Add last word
    }

    String current_line = "";

    for (const String &word : words) {
      String new_line = current_line + (current_line.isEmpty() ? "" : " ") + word;
      int text_width = 0, text_height = 0, x1, y1;
      it.get_text_bounds(x, y_offset, new_line.c_str(), font, TextAlign::TOP_LEFT, &x1, &y1, &text_width, &text_height);

      if (text_width <= width) {
        current_line = new_line;
      } else {
        lines.push_back(current_line);
        current_line = word;
      }
    }

    if (current_line.length() > 0) {
      lines.push_back(current_line);
    }
  }

  // Step 3: Paging + Draw
  int total_pages = (lines.size() + lines_to_show - 1) / lines_to_show;
  int start_line = (current_portion % total_pages) * lines_to_show;
  int end_line = min(start_line + lines_to_show, (int)lines.size());

  int clear_height = line_height * lines_to_show;
  it.filled_rectangle(x, y, width, clear_height, color_black);

  y_offset = y;

  for (int i = start_line; i < end_line; i++) {
    it.printf(x, y_offset, font, color_white, TextAlign::TOP_LEFT, "%s", lines[i].c_str());
    y_offset += line_height;
  }
}

void draw_art_deco_frame(Display &it, BaseImage* star_image, int x, int y, int width, int height, int angle_size, int angle_space = 11) {
  // Star images in angles, first to avoid image on top of line drawing
  it.image(x + 10, y + 10, star_image);
  it.image(x + 10, height - 10, star_image, ImageAlign::BOTTOM_LEFT);
  it.image(width - 10, y + 10, star_image, ImageAlign::TOP_RIGHT);

  // Frames
  it.rectangle(x, x, width, height);
  it.rectangle(x + 1, y + 1, width - 2, height - 2);
  it.rectangle(x + 2, y + 2, width - 4, height - 4);
  it.rectangle(x + 6, y + 6, width - 12, height - 12);

  // Top left corner
  it.line(x + angle_space + angle_size, y + angle_space, x + angle_space, y + angle_space + angle_size);
  it.line(x + angle_space + angle_size, y + angle_space, x + angle_space + (angle_size * 3), y + angle_space);
  it.line(x + angle_space, y + angle_space + angle_size, x + angle_space, y + angle_space + (angle_size * 3));

  // Bottom left corner
  it.line(x + angle_space + angle_size, y + height - angle_space, x + angle_space, y + height - angle_space - angle_size);
  it.line(x + angle_space + angle_size, y + height - angle_space, x + angle_space + (angle_size * 3), y + height - angle_space);
  it.line(x + angle_space, y + height - angle_space - angle_size, x + angle_space, y + height - angle_space - (angle_size * 3));

  // Top right corner
  it.line(x + width - angle_space - angle_size, y + angle_space, x + width - angle_space, y + angle_space + angle_size);
  it.line(x + width - angle_space - angle_size, y + angle_space, x + width - angle_space - (angle_size * 3), y + angle_space);
  it.line(x + width - angle_space, y + angle_space + angle_size, x + width - angle_space, y + angle_space + (angle_size * 3));

  // Bottom right corner
  it.line(x + width - angle_space - angle_size, y + height - angle_space, x + width - angle_space, y + height - angle_space - angle_size);
  it.line(x + width - angle_space - angle_size, y + height - angle_space, x + width - angle_space - (angle_size * 3), y + height - angle_space);
  it.line(x + width - angle_space, y + height - angle_space - angle_size, x + width - angle_space, y + height - angle_space - (angle_size * 3));
}

void draw_art_deco_title(Display &it, int x, int y, BaseFont* font, Color color_white, const char *text) {
  String title = text;
  int margin = 10;

  int text_width = 0, text_height = 0, x1, y1;
  it.get_text_bounds(x, y, title.c_str(), font, TextAlign::TOP_CENTER, &x1, &y1, &text_width, &text_height);

  int middle = y1 + text_height / 2 - 14;
  int x_offset = x1 - 4;

  it.line(x_offset - 50, middle, x_offset - margin, middle);
  it.line(x_offset - 50, middle + 1, x_offset - margin, middle + 1);
  it.line(x_offset - 50, middle - 1, x_offset - margin, middle - 1);

  it.line(x_offset - 40, middle + 9, x_offset - margin, middle + 7);
  it.line(x_offset - 40, middle - 9, x_offset - margin, middle - 7);

  it.line(x_offset - 30, middle + 15, x_offset - margin, middle + 13);
  it.line(x_offset - 30, middle - 15, x_offset - margin, middle - 13);

  it.line(x_offset + text_width + margin, middle, x_offset + text_width + 50, middle);
  it.line(x_offset + text_width + margin, middle + 1, x_offset + text_width + 50, middle + 1);
  it.line(x_offset + text_width + margin, middle - 1, x_offset + text_width + 50, middle - 1);

  it.line(x_offset + text_width + margin, middle + 7, x_offset + text_width + 40, middle + 9);
  it.line(x_offset + text_width + margin, middle - 7, x_offset + text_width + 40, middle - 9);

  it.line(x_offset + text_width + margin, middle + 13, x_offset + text_width + 30, middle + 15);
  it.line(x_offset + text_width + margin, middle - 13, x_offset + text_width + 30, middle - 15);

  it.printf(x, y, font, color_white, TextAlign::TOP_CENTER, title.c_str());
}

void draw_art_deco_separator(Display &it, BaseImage* star_image, int x, int y, int width, int height) {
  int image_size = 20;

  it.line(x + 30, y - 5, x + width - 30, y - 5);
  it.line(x + 30, y + height + 5, x + width - 30, y + height + 5);

  it.filled_rectangle(x, y, width, height);

  it.image(x + width / 2 - image_size / 2, y + height / 2 - image_size / 2, star_image);
}
