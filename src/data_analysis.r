library(ggplot2)
library(dplyr)
library(patchwork)
library(tidyr)

df <- read.csv("times.csv")
df_mpi <- read.csv("times_mpi.csv")

speedup_data <- df %>%
  filter(kernel_size == 11) %>%
  filter(problem_size > 128) %>%
  
  left_join(
    df %>% 
      filter(threads == 1) %>%
      select(problem_size, kernel_size, time_serial = time),
    by = c("problem_size", "kernel_size")
  ) %>%
  mutate(speedup = time_serial / time,
         efficiency = speedup / threads,
         method = "OpenMP") %>%
  filter(threads != 1)
print(speedup_data)

# MPI
speedup_data_mpi <- df_mpi %>%
  filter(kernel_size == 25) %>%
  filter(problem_size > 128) %>%
  filter(threads <= 128) %>%
  left_join(
    df %>% 
      filter(threads == 1) %>%
      select(problem_size, kernel_size, time_serial = time),
    by = c("problem_size", "kernel_size")
  ) %>%
  mutate(speedup = time_serial / time,
         efficiency = speedup / threads,
         method = "MPI") %>%
  filter(threads != 1)
print(speedup_data_mpi)

efficiency_matrix <- speedup_data %>%
  select(threads, problem_size, efficiency) %>%
  pivot_wider(names_from = problem_size, values_from = efficiency)

print(efficiency_matrix)

custom_colors <- c("#1E88E5", "#FFC107", "#004D40", "#0E2954",
                   "#E53935", "#8E24AA", "#43A047", "#FB8C00")

# Speedup plot
p1 <- ggplot(speedup_data, aes(x = threads, y = speedup, 
                               color = factor(problem_size),
                               group = problem_size)) +
  geom_line(linewidth = 1) +
  geom_point(size = 3) +
  scale_x_continuous(trans = 'log2', breaks = c(2,4,8,16,32,64)) +
  scale_color_manual(values = custom_colors) +
  labs(
       x = "Number of Threads",
       y = "Speedup",
       color = "Matrix Size") +
  guides(color = guide_legend()) +  # Ensure legend is visible
  theme_minimal() +
  theme(legend.position = c(0.05, 0.95),  # Top-left inside graph
        legend.justification = c(0, 1),
        legend.box.background = element_rect(color = "black", fill = "white"),
        legend.title = element_text(size = 16),
        legend.text = element_text(size = 14),
        axis.title = element_text(size = 16, face = "plain"),
        panel.border = element_rect(color = "black", fill = NA, linewidth = 1),
        panel.grid.major = element_line(linewidth = 0.5, color = "grey65"))

# Efficiency plot
p2 <- ggplot(speedup_data, aes(x = threads, y = efficiency, 
                               color = factor(problem_size),
                               group = problem_size)) +
  geom_line(linewidth = 1) +
  geom_point(size = 3) +
  scale_x_continuous(trans = 'log2', breaks = c(2,4,8,16,32,64,128)) +
  scale_y_continuous(breaks = seq(0, 1.8, by = 0.2), limits = c(0.5, 1.1)) +  # More values on Y-axis
  scale_color_manual(values = custom_colors) +
  labs(
       x = "Number of Processes",
       y = "Efficiency",
       color = "Matrix Size") +
  guides(color = guide_legend()) +  # Ensure legend is visible
  theme_minimal() +
  theme(legend.position = c(0.01, 0.99),  # Top-right inside graph
        legend.justification = c(0, 1),
        legend.title = element_text(size = 16),
        legend.box.background = element_rect(color = "black", fill = "white"),
        legend.text = element_text(size = 14),
        axis.title = element_text(size = 16, face = "plain"),
        panel.border = element_rect(color = "black", fill = NA, linewidth = 1),
        panel.grid.major = element_line(linewidth = 0.5, color = "grey65"))  # Added frame

# Arrange plots side by side
p1
p2

combined_data <- bind_rows(speedup_data, speedup_data_mpi)

# Verify the structure
str(combined_data)

# Create the efficiency comparison plot (example)
custom_colors <- c("OpenMP" = "#1F78B4", "MPI" = "#E31A1C")

# To create the kernel graphs:
df <- read.csv("times.csv")

kernel_data <- df %>%
  filter(problem_size == 8192, threads != 1) %>%
  left_join(
    df %>% filter(problem_size == 8192, threads == 1) %>% select(kernel_size, time_serial = time),
    by = "kernel_size"
  ) %>%
  mutate(speedup = time_serial / time,
         efficiency = speedup / threads)


custom_colors <- c("3" = "#1F78B4", "11" = "#33A02C", "25" = "#E31A1C", 
                   "51" = "#FF7F00", "101" = "#6A3D9A", "default1" = "#B15928", 
                   "default2" = "#A6CEE3", "default3" = "#FB9A99")

p_speedup <- ggplot(kernel_data, aes(x = threads, y = speedup, 
                                     color = factor(kernel_size))) +
  geom_line(linewidth = 0.9, alpha = 0.9) +
  geom_point(size = 3) +
  scale_x_continuous(trans = 'log2', breaks = c(2, 4, 8, 16, 32, 64)) +
  scale_y_continuous(expand = expansion(mult = c(0, 0.05)), limits = c(0,60)) +
  scale_color_manual(values = custom_colors) +
  labs(
       x = "Number of Threads",
       y = "Speedup",
       color = "Kernel Size") +
  theme_minimal(base_size = 14) +
  theme(
    panel.border = element_rect(color = "black", fill = NA, linewidth = 1),
    legend.position = c(0.01, 0.99),
    legend.justification = c(0, 1),
    legend.box.background = element_rect(color = "black", fill = "white", linewidth = 0.5),
    legend.text = element_text(size = 10),
    axis.title = element_text(size = 14, face = "plain"),
    plot.title = element_text(size = 16, face = "plain"),
    legend.title = element_text(size = 16),
    panel.grid.major = element_line(linewidth = 0.5, color = "grey65")
  )

p_speedup

p_efficiency <- ggplot(kernel_data, aes(x = threads, y = efficiency, 
                                        color = factor(kernel_size))) +
  geom_line(linewidth = 0.9, alpha = 0.9) +
  geom_point(size = 3) +
  scale_x_continuous(trans = 'log2', breaks = c(2,4,8,16,32,64)) +
  scale_y_continuous(breaks = seq(0, 1.8, by = 0.2), limits = c(0.4, 1.1)) +
  scale_color_manual(values = custom_colors) +
  labs(
       x = "Number of Threads",
       y = "Efficiency",
       color = "Kernel Size") +
  theme_minimal(base_size = 14) +
  theme(
    panel.border = element_rect(color = "black", fill = NA, linewidth = 1),
    legend.position = c(0.99, 0.99),
    legend.justification = c(1, 1),
    legend.box.background = element_rect(color = "black", fill = "white", linewidth = 0.5),
    legend.text = element_text(size = 10),
    axis.title = element_text(size = 14, face = "plain"),
    plot.title = element_text(size = 16, face = "plain"),
    legend.title = element_text(size = 16),
    panel.grid.major = element_line(linewidth = 0.5, color = "grey65")
  )

p_efficiency


# Comparison graph
p_combined <- ggplot(combined_data, aes(x = threads, y = efficiency)) +
  geom_line(aes(group = interaction(problem_size, method), color = method), 
            linewidth = 0.9, alpha = 0.3, show.legend = FALSE) +
  stat_smooth(aes(color = method, group = method), 
              method = "loess", se = FALSE, linewidth = 1.5, show.legend = TRUE) +
  scale_x_continuous(trans = 'log2', breaks = c(2, 4, 8, 16, 32, 64)) +
  scale_y_continuous(breaks = seq(0, 1.8, by = 0.2), limits = c(0, 1.1)) +
  scale_color_manual(values = custom_colors) +
  labs(
       x = "Number of Processes",
       y = "Efficiency",
       color = "Method") +
  theme_minimal(base_size = 14) +
  theme(
    panel.border = element_rect(color = "black", fill = NA, linewidth = 1),
    legend.position = c(0.99, 0.99),
    legend.justification = c(1, 1),
    legend.title = element_text(size = 16),
    legend.box.background = element_rect(color = "black", fill = "white", linewidth = 0.5),
    legend.text = element_text(size = 14),
    axis.title = element_text(size = 16, face = "plain"),
    panel.grid.major = element_line(linewidth = 0.5, color = "grey65")
  )

p_combined