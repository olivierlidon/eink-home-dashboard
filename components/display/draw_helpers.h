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
