from Tkinter import *
import Tkinter, tkFileDialog

class Chooser:
    def __init__(self, master):
        self._720p = IntVar()
        self._480p = IntVar()
        self.filename = StringVar()

        check_720p = Checkbutton(master, text="720p", variable=self._720p, command=self.cb)
        check_720p.pack(fill=X, padx=10)

        check_480p = Checkbutton(master, text="480p", variable=self._480p, command=self.cb)
        check_480p.pack(fill=X, padx=10, pady=10)

        button_choose = Button(master, text='Choose File', command=self.choose_file)
        button_choose.pack(fill=X, padx=10, pady=40)

        button_start = Button(master, text='Start', command=self.start)
        button_start.pack(fill=X, padx=10)

    def cb(self):
        print "720p is", self._720p.get()
        print "480p is", self._480p.get()

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
