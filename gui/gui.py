from Tkinter import *
import Tkinter, tkFileDialog

class Chooser:
    def __init__(self, master):
        self.resolution = StringVar()
        self.resolution.set("360p")
        self.filename = StringVar()

        label = Label(master, text="Select Target Resolution")
        label.pack(fill=X, padx=10, pady=10)

        option_res = OptionMenu(master, self.resolution, "360p", "480p", "720p", command=self.selected)
        option_res.pack(padx=10, pady=10)

        button_choose = Button(master, text='Choose File', command=self.choose_file)
        button_choose.pack(fill=X, padx=10, pady=40)

        button_start = Button(master, text='Start', command=self.start)
        button_start.pack(fill=X, padx=5)

    def selected(self, res):
        print "Resolution " + res + " is chosen."

    def choose_file(self):
        self.filename = tkFileDialog.askopenfilename(initialdir = "/home",  filetypes=[('All', '*'), ('mp4','*.mp4'), ('mpeg', '*mpeg')])
        print self.filename

    def start(self):
        pass


master = Tkinter.Tk()
master.minsize(width=320, height=240)
master.title("Video Resolution Adjuster")
chooser = Chooser(master)

mainloop()
