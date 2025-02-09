import pygame

# Initialize pygame
pygame.init()

# Constants
WIDTH, HEIGHT = 500, 150
BAR_HEIGHT = 50
POINT_RADIUS = 8
WHITE = (255, 255, 255)

# Colors
color_left = (0, 255, 0)  # Green
color_right = (0, 0, 255)  # Blue

# Create the window
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Color Gradient with Weighting Point")

# Weighting point position
point_x = int(WIDTH * 0.75)
point_dragging = False

def calculate_weighted_color(x, width, color1, color2):
    """Calculates the gradient color based on the weighting point's position."""
    weight = x / width  # Normalize weight (0 to 1)
    r = int((1 - weight) * color1[0] + weight * color2[0])
    g = int((1 - weight) * color1[1] + weight * color2[1])
    b = int((1 - weight) * color1[2] + weight * color2[2])
    return (r, g, b)

def draw_gradient(screen, width, height, weight_x):
    """Draws a weighted gradient between two colors."""
    for x in range(width):
        color = calculate_weighted_color(x + (weight_x - width // 2), width, color_left, color_right)
        pygame.draw.line(screen, color, (x, height // 2 - BAR_HEIGHT // 2), (x, height // 2 + BAR_HEIGHT // 2))

def main():
    global point_x, point_dragging
    running = True
    
    while running:
        screen.fill(WHITE)
        
        # Event handling
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.MOUSEBUTTONDOWN:
                if event.button == 1:  # Left click
                    if abs(event.pos[0] - point_x) < POINT_RADIUS * 2:
                        point_dragging = True
            elif event.type == pygame.MOUSEBUTTONUP:
                if event.button == 1:
                    point_dragging = False
            elif event.type == pygame.MOUSEMOTION:
                if point_dragging:
                    point_x = max(0, min(WIDTH, event.pos[0]))
        
        # Draw gradient
        draw_gradient(screen, WIDTH, HEIGHT, point_x)
        
        # Draw weighting point
        pygame.draw.polygon(screen, color_left, [(0, HEIGHT // 2 - BAR_HEIGHT // 2), (0, HEIGHT // 2 + BAR_HEIGHT // 2), (20, HEIGHT // 2)])
        pygame.draw.polygon(screen, color_right, [(WIDTH, HEIGHT // 2 - BAR_HEIGHT // 2), (WIDTH, HEIGHT // 2 + BAR_HEIGHT // 2), (WIDTH - 20, HEIGHT // 2)])
        pygame.draw.circle(screen, (0, 100, 255), (point_x, HEIGHT // 2), POINT_RADIUS)
        
        pygame.display.flip()
    
    pygame.quit()

if __name__ == "__main__":
    main()
