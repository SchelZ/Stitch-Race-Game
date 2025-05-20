from panda3d.core import QueuedConnectionManager, loadPrcFileData, QueuedConnectionReader, ConnectionWriter, NetAddress, Datagram, NetDatagram, DatagramIterator, QueuedConnectionListener, PointerToConnection
from direct.task import Task
from direct.showbase.ShowBase import ShowBase

loadPrcFileData("", """
window-type none
audio-library-name null
framebuffer-hardware false
sync-video false
""")

class PandaServer(ShowBase):
    def __init__(self, port: int = 5000, backlog: int = 1000):
        ShowBase.__init__(self)

        self.manager = QueuedConnectionManager()
        self.reader = QueuedConnectionReader(self.manager, 0)
        self.writer = ConnectionWriter(self.manager, 0)
        self.listener = QueuedConnectionListener(self.manager, 0)
        self.connections = []

        self.tcp_socket = self.manager.openTCPServerRendezvous(port, backlog)
        self.listener.addConnection(self.tcp_socket)

        self.taskMgr.add(self.onNewConnection, "Poll the connection listener", -39)
        self.taskMgr.add(self.onReceiveData, "Poll the connection reader", -40)
        self.taskMgr.add(self.check_disconnections, "Check disconnects", -41)

        print(f"[Server] Listening on port {port}...")

    def check_disconnections(self, task):
        if self.manager.resetConnectionAvailable():
            ptr = PointerToConnection()
            if self.manager.getResetConnection(ptr):
                connection = ptr.p()
                self.onDisconnect(connection)
        return Task.cont

    def onNewConnection(self, taskdata):
        if self.listener.newConnectionAvailable():
            rendezvous = PointerToConnection()
            netAddress = NetAddress()
            newConnection = PointerToConnection()

            if self.listener.getNewConnection(rendezvous, netAddress, newConnection):
                conn = newConnection.p()
                self.connections.append(conn)
                self.reader.addConnection(conn)
                print(f"[New Connection] From {netAddress.getIpString()}:{netAddress.getPort()}")
        return Task.cont


    def onReceiveData(self, taskdata):
        if self.reader.dataAvailable():
            datagram = NetDatagram()
            if self.reader.getData(datagram):
                self.processData(datagram)
        return Task.cont
    
    
    def onDisconnect(self, connection):
        print(f"[Disconnect] A client has disconnected.")
        if connection in self.connections:
            self.connections.remove(connection)
        self.reader.removeConnection(connection)
        self.manager.closeConnection(connection)

    def processData(self, datagram):
        di = DatagramIterator(datagram)

        try:
            name = di.get_string()
            car_id = di.get_int32()
            color_id = di.get_int32()

            pos = (di.get_float32(), di.get_float32(), di.get_float32())
            hpr = (di.get_float32(), di.get_float32(), di.get_float32())
            scale = (di.get_float32(), di.get_float32(), di.get_float32())

            print(f"\n[Received Player Data]")
            print(f"  Name     : {name}")
            print(f"  Color ID : {color_id}")
            print(f"  Position : {pos}")
            print(f"  Rotation : {hpr}")
            print(f"  Scale    : {scale}")

        except Exception as e:
            print(f"[Error parsing datagram] {e}")


if __name__ == "__main__":
    PandaServer(port=5000).run()
