library(ggplot2)
library(dplyr)

# Read data
df <- read.csv("times.csv")

# Filter data for kernel_size = 25 and calculate speedup
speedup_data <- df %>%
  filter(kernel_size == 25) %>%
  # Get single-thread times as baseline
  left_join(
    df %>% 
      filter(threads == 1) %>%
      select(problem_size, kernel_size, time_serial = time),
    by = c("problem_size", "kernel_size")
  ) %>%
  # Calculate speedup (T_serial / T_parallel)
  mutate(speedup = time_serial / time) %>%
  filter(threads != 1)  # Remove single-thread entries

# Create plot
ggplot(speedup_data, aes(x = threads, y = speedup, 
                         color = factor(problem_size),
                         group = problem_size)) +
  geom_line(linewidth = 1) +
  geom_point(size = 2) +
  scale_x_continuous(trans = 'log2', breaks = c(2,4,8,16,32,64)) +
  labs(title = "Speedup for Kernel Size 25",
       subtitle = "Comparison Across Matrix Dimensions",
       x = "Number of Threads",
       y = "Speedup (T_serial / T_parallel)",
       color = "Matrix Size") +
  theme_minimal() +
  theme(legend.position = "bottom")