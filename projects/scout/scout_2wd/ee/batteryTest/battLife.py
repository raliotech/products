from numpy import genfromtxt
my_data = genfromtxt('scoutDanceBatt_4.csv', delimiter=',')[1:,]

from bokeh.io import output_file, show, export_png
from bokeh.plotting import figure
from bokeh.layouts import column



output_file("layout.html")

x = my_data[:, 0].reshape((1, 50000))[0]
y1 = my_data[:, 2].reshape((1, 50000))[0]
y2 = my_data[:, 3].reshape((1, 50000))[0]
y3 = my_data[:, 4].reshape((1, 50000))[0]
y4 = my_data[:, 5].reshape((1, 50000))[0]

fig0 = figure(title='scoutDanceBatt_tof', height = 40)

fig1 = figure(title = 'Vin vs t', x_axis_label = 't ms', y_axis_label = 'V', width = 1750, height = 250)
fig1.line(x, y1, color='firebrick')
fig1.line([0, 500000], [5.5, 5.5], color='firebrick', width=2)

fig2 = figure(title = '5V vs t', x_axis_label = 't ms', y_axis_label = 'V', width = 1750, height = 250)
fig2.line(x, y2, color='olive')
fig2.line([0, 500000], [3.5, 3.5], color='olive', width=2)

fig3 = figure(title = '3V3 vs t', x_axis_label = 't ms', y_axis_label = 'V', width = 1750, height = 250)
fig3.line(x, y3, color='green')
fig3.line([0, 500000], [1.7, 1.7], color='green', width=2)

fig4 = figure(title = 'Curr vs t', x_axis_label = 't ms', y_axis_label = 'mA', width = 1750, height = 250)
fig4.line(x, y4)
fig4.line([0, 500000], [200, 200], width=2)

show(column(fig0, fig1, fig2, fig3, fig4))
