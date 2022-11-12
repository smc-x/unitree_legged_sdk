package main

import (
	"github.com/eiannone/keyboard"
	zmq "github.com/pebbe/zmq4"
	"github.com/sirupsen/logrus"
)

func main() {
	ctx, err := zmq.NewContext()
	panicIf(err)

	sub, err := ctx.NewSocket(zmq.SUB)
	panicIf(err)
	defer errCall(sub.Close)
	panicIf(sub.SetSubscribe("stat"))

	pub, err := ctx.NewSocket(zmq.PUB)
	panicIf(err)
	defer errCall(pub.Close)

	panicIf(sub.Connect("tcp://192.168.123.15:5555"))
	panicIf(pub.Connect("tcp://192.168.123.15:5556"))

	go func() {
		for {
			msg, err := sub.Recv(0)
			if err != nil {
				logger.Error(err)
			} else {
				logger.Infof("%s", msg)
			}
		}
	}()

	panicIf(keyboard.Open())
	defer errCall(keyboard.Close)
	for {
		_, key, err := keyboard.GetKey()
		panicIf(err)

		switch key {
		case keyboard.KeyArrowUp:
			// forward
			_, err = pub.Send("ctrl 0.5 0 0", 0)
			logIf(err)
		case keyboard.KeyArrowDown:
			// backward
			_, err = pub.Send("ctrl -0.5 0 0", 0)
			logIf(err)
		case keyboard.KeyArrowLeft:
			// turn left
			_, err = pub.Send("ctrl 0 0 0.5", 0)
			logIf(err)
		case keyboard.KeyArrowRight:
			// turn right
			_, err = pub.Send("ctrl 0 0 -0.5", 0)
			logIf(err)
		case keyboard.KeyTab:
			// horizontally left
			_, err = pub.Send("ctrl 0 0.5 0", 0)
			logIf(err)
		case keyboard.KeySpace:
			// horizontally right
			_, err = pub.Send("ctrl 0 -0.5 0", 0)
			logIf(err)
		}

		if key == keyboard.KeyEsc {
			break
		}
	}
}

var (
	logger = logrus.WithField("n", "unitree-client")
)

func logIf(err error) {
	if err != nil {
		logger.Error(err)
	}
}

func panicIf(err error) {
	if err != nil {
		panic(err)
	}
}

func errCall(cb func() error) {
	if err := cb(); err != nil {
		logger.Error(err)
	}
}
