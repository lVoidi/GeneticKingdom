        // Calcular radio en píxeles
        float radius = static_cast<float>(range * cellSize);
        
        // Dibujar círculo semitransparente
// ... (código intermedio) ...
        // Calcular las celdas dentro del rango
        for (int r_loop = row - range; r_loop <= row + range; r_loop++) {
            for (int c_loop = col - range; c_loop <= col + range; c_loop++) {
                // Calcular distancia euclidiana
                float dx = static_cast<float>(c_loop - col);
                float dy = static_cast<float>(r_loop - row);
                float distance = sqrtf(dx * dx + dy * dy);
                
                // Si está dentro del rango
                if (distance <= static_cast<float>(range)) {
                    // Dibujar borde de la celda
                    graphics.DrawRectangle(&pen, static_cast<float>(c_loop * cellSize), static_cast<float>(r_loop * cellSize), static_cast<float>(cellSize), static_cast<float>(cellSize));
                }
            }
        } 