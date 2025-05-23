import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os

output_dir = "plots"
if not os.path.exists(output_dir):
    os.makedirs(output_dir)

df = pd.read_csv("build/wave_solution_data.csv")

plt.figure(figsize=(12, 8))

time_columns = [col for col in df.columns if col != 'x']

colors = plt.cm.viridis(np.linspace(0, 1, len(time_columns)))

for i, col in enumerate(time_columns):
    time_step = int(col.split('_')[1])
    time_value = time_step / len(time_columns) * 1.0  # T=1.0
    
    plt.plot(df['x'], df[col], color=colors[i], 
             label=f't = {time_value:.2f}', linewidth=2)

plt.title('Transport Equation Solution at Different Time Steps')
plt.xlabel('x')
plt.ylabel('u(t,x)')
plt.grid(True)
plt.legend(loc='best')

plot_path = os.path.join(output_dir, "transport_solution_multiple_times.png")
plt.savefig(plot_path, dpi=300)
plt.close()

print(f"Plot saved to {plot_path}")

fig = plt.figure(figsize=(14, 10))
ax = fig.add_subplot(111, projection='3d')

x_values = df['x'].values
time_values = np.linspace(0, 1.0, len(time_columns))

X, T = np.meshgrid(x_values, time_values)
Z = np.zeros_like(X)

for i, col in enumerate(time_columns):
    Z[i, :] = df[col].values

surf = ax.plot_surface(X, T, Z, cmap='viridis', edgecolor='none', alpha=0.8)

ax.set_xlabel('x')
ax.set_ylabel('t')
ax.set_zlabel('u(t,x)')
ax.set_title('Transport Equation Solution (3D View)')

fig.colorbar(surf, ax=ax, shrink=0.5, aspect=5)

plot_path_3d = os.path.join(output_dir, "transport_solution_3d.png")
plt.savefig(plot_path_3d, dpi=300)
plt.close()

print(f"3D plot saved to {plot_path_3d}")